#include "device.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

namespace
{
// simple utility function to ensure the first parameter is actually a symbol known to the compiler
template<typename T>
constexpr auto identityCompileCheck(T, const char* s)
{
  return s;
}
} // namespace

#define EE_STRINGIFY(x) identityCompileCheck(x, #x)

namespace audio
{
namespace
{
const std::array<ALCint, 5> deviceQueryParamList{ALC_STEREO_SOURCES, 64, ALC_SYNC, ALC_FALSE, ALC_INVALID};

void logDeviceInfo(ALCdevice* device)
{
  BOOST_LOG_TRIVIAL(info) << "OpenAL device: " << alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
  BOOST_LOG_TRIVIAL(info) << "OpenAL version: " << AL_ASSERT_FN(alGetString(AL_VERSION));
  BOOST_LOG_TRIVIAL(info) << "OpenAL vendor: " << AL_ASSERT_FN(alGetString(AL_VENDOR));
  BOOST_LOG_TRIVIAL(info) << "OpenAL renderer: " << AL_ASSERT_FN(alGetString(AL_RENDERER));

  {
    ALCint frq;
    ALC_ASSERT(device, alcGetIntegerv(device, ALC_FREQUENCY, 1, &frq));
    BOOST_LOG_TRIVIAL(info) << "OpenAL: Frequency " << frq;
  }

  if(alcIsExtensionPresent(device, EE_STRINGIFY(ALC_SOFT_HRTF)) != ALC_TRUE)
  {
    BOOST_LOG_TRIVIAL(info) << "ALC_SOFT_HRTF extension not present";
  }
  else
  {
    ALCint value = 0;
    ALC_ASSERT(device, alcGetIntegerv(device, ALC_HRTF_STATUS_SOFT, 1, &value));
    switch(value)
    {
    case ALC_HRTF_DISABLED_SOFT: BOOST_LOG_TRIVIAL(info) << "HRTF is disabled"; break;
    case ALC_HRTF_ENABLED_SOFT: BOOST_LOG_TRIVIAL(info) << "HRTF is enabled"; break;
    case ALC_HRTF_DENIED_SOFT: BOOST_LOG_TRIVIAL(info) << "HRTF is not allowed on this device"; break;
    case ALC_HRTF_REQUIRED_SOFT: BOOST_LOG_TRIVIAL(info) << "HRTF is required on this device"; break;
    case ALC_HRTF_HEADPHONES_DETECTED_SOFT:
      BOOST_LOG_TRIVIAL(info) << "HRTF is enabled because headphones were detected";
      break;
    case ALC_HRTF_UNSUPPORTED_FORMAT_SOFT:
      BOOST_LOG_TRIVIAL(info) << "HRTF is disabled because of an incompatible format";
      break;
    default: BOOST_LOG_TRIVIAL(warning) << "Unknown HRTF status value #" << value; break;
    }
  }
}

std::optional<std::tuple<ALCdevice*, ALCcontext*>> tryCreateDeviceContext(const char* deviceName)
{
  BOOST_LOG_TRIVIAL(info) << "Probing device " << deviceName;
  auto device = alcOpenDevice(deviceName);

  if(device == nullptr)
  {
    BOOST_LOG_TRIVIAL(info) << "Failed to open device";
    return std::nullopt;
  }

  if(alcIsExtensionPresent(device, ALC_EXT_EFX_NAME) == ALC_TRUE)
  {
    BOOST_LOG_TRIVIAL(info) << "Device supports EFX";
    auto context = alcCreateContext(device, deviceQueryParamList.data());
    // fails e.g. with Rapture3D, where EFX is supported
    if(context != nullptr)
    {
      return std::tuple{device, context};
    }
  }

  alcCloseDevice(device);
  return std::nullopt;
}
} // namespace

Device::~Device()
{
  m_shutdown = true;
  m_streamUpdater.join();

  m_underwaterFilter.reset();

  m_sources.clear();
  {
    std::lock_guard lock{m_streamsLock};
    m_streams.clear();
  }

  if(m_context != nullptr)
  {
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(m_context);
  }

  if(m_device != nullptr)
  {
    alcCloseDevice(m_device);
  }
}

Device::Device()
{
  alcGetError(nullptr); // clear any error

  if(alcIsExtensionPresent(nullptr, EE_STRINGIFY(ALC_ENUMERATE_ALL_EXT)) == ALC_TRUE)
  {
    BOOST_LOG_TRIVIAL(info) << "Probing OpenAL devices...";

    gsl::czstring deviceList = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
    Expects(deviceList != nullptr);

    if(deviceList == nullptr)
    {
      BOOST_LOG_TRIVIAL(warning) << "No audio devices";
      return;
    }

    while(*deviceList != '\0')
    {
      if(const auto deviceContext = tryCreateDeviceContext(deviceList))
      {
        m_device = std::get<0>(*deviceContext);
        m_context = std::get<1>(*deviceContext);
        break;
      }
      deviceList += strlen(deviceList) + 1;
    }
  }
  else
  {
    BOOST_LOG_TRIVIAL(info) << "Trying to use default OpenAL device";
    m_device = alcOpenDevice(nullptr);

    if(m_device != nullptr)
    {
      m_context = alcCreateContext(m_device, deviceQueryParamList.data());
    }
    else
    {
      BOOST_LOG_TRIVIAL(error) << "Failed to open default device";
    }
  }

  if(m_context == nullptr)
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to create OpenAL context.";
    if(m_device != nullptr)
      alcCloseDevice(m_device);
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create OpenAL context."));
  }

  alcMakeContextCurrent(m_context);
  AL_ASSERT(alListenerf(AL_METERS_PER_UNIT, 1 / 512.0f));
  AL_ASSERT(alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED));

  loadALExtFunctions(m_device);

  logDeviceInfo(m_device);

  m_underwaterFilter = std::make_shared<FilterHandle>();

  AL_ASSERT(alFilteri(m_underwaterFilter->get(), AL_FILTER_TYPE, AL_FILTER_LOWPASS));
  AL_ASSERT(alFilterf(m_underwaterFilter->get(), AL_LOWPASS_GAIN, 0.7f));   // Low frequencies gain.
  AL_ASSERT(alFilterf(m_underwaterFilter->get(), AL_LOWPASS_GAINHF, 0.1f)); // High frequencies gain.

  m_streamUpdater = std::thread{[this]()
                                {
                                  while(!this->m_shutdown)
                                  {
                                    this->updateStreams();
                                    std::this_thread::yield();
                                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                                  }
                                }};
#ifdef _WIN32
  if(FAILED(SetThreadDescription(m_streamUpdater.native_handle(), L"device stream updater")))
    BOOST_LOG_TRIVIAL(warning) << "Failed to set thread description for audio updater thread";
#endif
}

void Device::reset()
{
  {
    std::lock_guard lock{m_streamsLock};
    for(const auto& stream : m_streams)
    {
      stream->setLooping(false);
      stream->stop();
    }
  }

  std::lock_guard lock{m_streamsLock};
  m_streams.clear();
  m_sources.clear();
}
} // namespace audio
