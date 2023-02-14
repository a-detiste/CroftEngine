#pragma once

#include <algorithm>
#include <cstdint>
#include <gsl/gsl-lite.hpp>

constexpr uint8_t clampu8(int32_t val)
{
  return gsl::narrow_cast<uint8_t>(std::clamp(val, 0, 255));
}

template<typename T>
constexpr T sq(T val)
{
  return val * val;
}
