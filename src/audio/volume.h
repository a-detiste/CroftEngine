#include <AL/al.h>
#include <algorithm>
#include <cmath>

namespace audio
{
inline ALfloat toLogarithmicVolumeExact(ALfloat v)
{
  return std::pow(1000.0f, std::max(0.0f, v) - 1);
}
} // namespace audio
