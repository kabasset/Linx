// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_INTERVAL_FUNCS_H
#define LINX_BASE_INTERVAL_FUNCS_H

#include <Kokkos_Core.hpp>
#include <concepts>

namespace Linx {

/**
 * @brief Integral slice iterator.
 */
template <std::integral T>
class SliceIterator {
public:

  using value_type = T; ///< The value type
  using pointer = const T*; ///< The constant pointer type
  using reference = const T&; ///< The constant pointer type
  using difference_type = std::ptrdiff_t; ///< The address difference type

  explicit SliceIterator(const value_type& current) : m_current(current) {}

  /**
   * @brief Dereference operator.
   */
  constexpr reference operator*() const
  {
    return m_current;
  }

  /**
   * @brief Arrow operator.
   */
  constexpr pointer operator->() const
  {
    return &m_current;
  }

  /**
   * @brief Prefix increment operator.
   */
  constexpr SliceIterator& operator++()
  {
    ++m_current;
    return *this;
  }

  /**
   * @brief Postfix increment operator.
   */
  constexpr SliceIterator operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  /**
   * @brief Equality operator.
   */
  constexpr bool operator==(const SliceIterator& other) const
  {
    return m_current == other.m_current;
  };

private:

  T m_current; ///< Non-const value
};

/**
 * @brief Iterator to the beginning of an integral slice.
 */
template <std::integral T, typename TPred>
constexpr SliceIterator<T> begin(const Slice<T, TPred>& interval)
{
  return SliceIterator<T>(interval.start());
}

/**
 * @brief Iterator to the end of an integral slice.
 */
template <std::integral T, typename TPred>
constexpr SliceIterator<T> end(const Slice<T, TPred>& interval)
{
  return SliceIterator<T>(interval.stop());
}

/**
 * @brief Get the Kokkos execution policy of a slice.
 */
template <typename TSpace, std::integral T, typename TPred>
auto kokkos_execution_policy(const Slice<T, TPred>& region)
{
  return Kokkos::RangePolicy<TSpace, Kokkos::IndexType<Index>>(region.start(), region.stop());
}

/**
 * @ingroup regions
 * @brief Apply a function to each element of the domain.
 * @tparam TSpace The execution space
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, std::integral T, typename TPred>
void for_each(const std::string& label, const Slice<T, TPred>& slice, auto&& func)
{
  Kokkos::parallel_for(label, kokkos_execution_policy<TSpace>(slice), LINX_FORWARD(func));
}

} // namespace Linx

#endif
