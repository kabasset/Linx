// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_MEDIAN_H
#define LINX_BASE_MEDIAN_H

#include "Linx/Base/Functional.h"
#include "Linx/Base/SelectNet.h"
#include "Linx/Base/Types.h"

#include <algorithm> // min
#include <numeric> // midpoint

namespace Linx {

/**
 * @brief Find the n-th element of an array.
 */
const auto& introselect_n(auto& in_out, std::integral auto n)
{
  auto begin = &in_out[0];
  auto nth = begin + n;
  auto end = begin + std::size(in_out);
  std::nth_element(begin, nth, end);
  return *nth;
}

/**
 * @brief Sort the n first elements of an array.
 */
const auto& heapselect_n(auto& in_out, std::integral auto n)
{
  auto begin = &in_out[0];
  auto nth = begin + n;
  auto end = begin + std::size(in_out);
  std::partial_sort(begin, nth, end);
  return *nth;
}

/**
 * @brief Sort the n first values of an array.
 * 
 * `std::nth_element()` typically relies on intro-select, and `std::partial_sort()` on heap-select.
 * Insertion-sort has higher asymptotic complexity but should be faster for small arrays,
 * which is typically the case for rank-filtering.
 */
KOKKOS_INLINE_FUNCTION constexpr const auto& insertsort_n(auto& in_out, std::integral auto n)
{
  using T = LINX_DECLTYPE(in_out[0]);
  T current;
  std::size_t j;
  for (std::size_t i = 0; i < std::size(in_out); ++i) {
    j = std::min<std::size_t>(i, n + 1);
    current = in_out[i];
    in_out[i] = in_out[j];
    for (; j > 0 && current < in_out[j - 1]; --j) {
      in_out[j] = in_out[j - 1];
    }
    in_out[j] = current;
  }
  return in_out[n];
}

/**
 * @brief Compute the median of an array of odd size.
 */
KOKKOS_INLINE_FUNCTION constexpr auto median_odd(auto& in_out)
{
  return insertsort_n(in_out, std::size(in_out) / 2);
}

/**
 * @brief Compute the median of an array of even size.
 */
KOKKOS_INLINE_FUNCTION constexpr auto median_even(auto& in_out)
{
  const auto& high = insertsort_n(in_out, std::size(in_out) / 2);
  const auto& low = *(&high - 1);
  return std::midpoint(low, high);
}

/**
 * @brief Compute the median of an array of unknown size and size parity.
 */
template <typename TArray>
KOKKOS_INLINE_FUNCTION constexpr auto median(TArray& in_out)
{
  if constexpr (requires { TArray::size() >= 0; }) {
    return median<TArray::size()>(in_out);
  } else {
    if (std::size(in_out) % 2 == 0) {
      return median_even(in_out);
    } else {
      return median_odd(in_out);
    }
  }
}

/**
 * @brief Compute the median of an array with static size.
 */
template <std::integral auto N>
KOKKOS_INLINE_FUNCTION constexpr auto median(auto& in_out)
{
  if constexpr (requires { SelectNet<N> {}; }) {
    return SelectNet<N>::median(in_out);
  } else if constexpr (N % 2 == 0) {
    return median_even(in_out);
  } else {
    return median_odd(in_out);
  }
}

} // namespace Linx

#endif
