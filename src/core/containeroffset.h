#pragma once

#include "serialization/serialization.h"
#include "tpl_helper.h"

#include <boost/throw_exception.hpp>
#include <stdexcept>
#include <vector>

namespace core
{
template<typename OffsetType, typename... DataTypes>
struct ContainerOffset
{
  static_assert(std::is_integral_v<OffsetType> && !std::is_signed_v<OffsetType>,
                "Index type must be unsigned integer like");
  static_assert(sizeof...(DataTypes) > 0, "Must provide at least one bound type");

  using offset_type = OffsetType;

  offset_type offset = 0;

  constexpr ContainerOffset() = default;

  // cppcheck-suppress noExplicitConstructor
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr ContainerOffset(offset_type offset)
      : offset{offset}
  {
  }

  template<typename T>
  explicit ContainerOffset(T) = delete;

  template<typename T>
  offset_type index() const
  {
    static_assert(tpl::contains_v<T, DataTypes...>, "Can only use declared types for index conversion");
    if(offset % sizeof(T) != 0)
      BOOST_THROW_EXCEPTION(std::runtime_error("Offset not dividable by element size"));

    return offset / sizeof(T);
  }

  template<typename T>
  [[nodiscard]] constexpr auto from(std::vector<T>& v) const -> std::enable_if_t<tpl::contains_v<T, DataTypes...>, T&>
  {
    if(offset % sizeof(T) != 0)
      BOOST_THROW_EXCEPTION(std::runtime_error("Offset not dividable by element size"));

    return v.at(offset / sizeof(T));
  }

  template<typename T>
  [[nodiscard]] constexpr auto from(const std::vector<T>& v) const
    -> std::enable_if_t<tpl::contains_v<T, DataTypes...>, const T&>
  {
    if(offset % sizeof(T) != 0)
      BOOST_THROW_EXCEPTION(std::runtime_error("Offset not dividable by element size"));

    return v.at(offset / sizeof(T));
  }
};

template<typename IndexType, typename... DataTypes>
struct ContainerIndex
{
  static_assert(std::is_integral_v<IndexType> && !std::is_signed_v<IndexType>,
                "Index type must be unsigned integer like");
  static_assert(sizeof...(DataTypes) > 0, "Must provide at least one bound type");

  using index_type = IndexType;

  index_type index = 0;

  constexpr ContainerIndex() = default;

  // cppcheck-suppress noExplicitConstructor
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr ContainerIndex(index_type index) noexcept
      : index{index}
  {
  }

  template<typename T>
  explicit ContainerIndex(T) = delete;

  template<typename T>
  [[nodiscard]] constexpr auto from(std::vector<T>& v) const -> std::enable_if_t<tpl::contains_v<T, DataTypes...>, T&>
  {
    return v.at(index);
  }

  template<typename T>
  [[nodiscard]] constexpr auto from(const std::vector<T>& v) const
    -> std::enable_if_t<tpl::contains_v<T, DataTypes...>, const T&>
  {
    return v.at(index);
  }

  auto& operator+=(index_type delta)
  {
    if((index > 0) && (delta > std::numeric_limits<index_type>::max() - index))
      BOOST_THROW_EXCEPTION(std::out_of_range("Index addition causes overflow"));

    index += delta;
    return *this;
  }

  [[nodiscard]] auto operator+(const ContainerIndex<IndexType, DataTypes...>& delta) const
  {
    if((index > 0) && (delta.index > std::numeric_limits<index_type>::max() - index))
      BOOST_THROW_EXCEPTION(std::out_of_range("Index addition causes overflow"));

    return index + delta.index;
  }

  template<typename T>
  void operator+=(T) = delete;

  void serialize(const serialization::Serializer& ser)
  {
    ser(S_NV("index", index));
  }

  static ContainerIndex<IndexType, DataTypes...> create(const serialization::Serializer& ser)
  {
    index_type tmp = 0;
    ser(S_NV("index", tmp));
    return {tmp};
  }
};
} // namespace core
