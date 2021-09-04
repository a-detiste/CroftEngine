#pragma once

#include "alext.h"
#include "utils.h"

namespace audio
{
class FilterHandle final
{
  const ALuint m_handle{};

  [[nodiscard]] static ALuint createHandle()
  {
    ALuint handle;
    AL_ASSERT(alGenFilters(1, &handle));

    Expects(alIsFilter(handle));

    return handle;
  }

public:
  explicit FilterHandle()
      : m_handle{createHandle()}
  {
  }

  explicit FilterHandle(const FilterHandle&) = delete;

  explicit FilterHandle(FilterHandle&&) = delete;

  FilterHandle& operator=(const FilterHandle&) = delete;

  FilterHandle& operator=(FilterHandle&&) = delete;

  ~FilterHandle()
  {
    AL_ASSERT(alDeleteFilters(1, &m_handle));
  }

  [[nodiscard]] ALuint get() const noexcept
  {
    return m_handle;
  }
};
} // namespace audio
