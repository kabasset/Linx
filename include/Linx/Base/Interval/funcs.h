// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_INTERVAL_FUNCS_H
#define LINX_BASE_INTERVAL_FUNCS_H

#include <Kokkos_Core.hpp>
#include <concepts>

namespace Linx {

template <std::integral T>
struct SliceIterator {
  using value_type = const T;
  using element_type = T;
  using pointer = value_type*;
  using reference = value_type&;
  using difference_type = std::ptrdiff_t;

  constexpr reference operator*() const
  {
    return value;
  }

  constexpr pointer operator->() const
  {
    return &value;
  }

  constexpr SliceIterator& operator++()
  {
    ++value;
    return *this;
  }

  constexpr SliceIterator operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  constexpr bool operator==(const SliceIterator& other) const
  {
    return value == other.value;
  };

  constexpr bool operator!=(const SliceIterator& other) const
  {
    return value != other.value;
  };

  element_type value;
};

template <std::integral T, typename TPred>
constexpr SliceIterator<T> begin(const Slice<T, TPred>& interval)
{
  return {interval.start()};
}

template <std::integral T, typename TPred>
constexpr SliceIterator<T> end(const Slice<T, TPred>& interval)
{
  return {interval.stop()};
}

/**
 * @brief Get the Kokkos execution policy of a slice.
 */
template <typename TSpace, std::integral T, typename TPred>
auto kokkos_execution_policy(const Slice<T, TPred>& region) // FIXME requires start(region), stop(region)
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
