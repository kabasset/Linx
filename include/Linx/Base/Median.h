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
KOKKOS_INLINE_FUNCTION const auto& insertsort_n(auto& in_out, std::integral auto n)
{
  using T = std::remove_cvref_t<decltype(in_out[0])>;
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
 * @brief Compute the median of an array of known size parity.
 */
template <typename TParity>
KOKKOS_INLINE_FUNCTION auto median(auto& in_out)
{
  if constexpr (std::is_same_v<TParity, OddNumber>) {
    return insertsort_n(in_out, std::size(in_out) / 2);
  } else if constexpr (std::is_same_v<TParity, EvenNumber>) {
    const auto& high = insertsort_n(in_out, std::size(in_out) / 2);
    const auto& low = *(&high - 1);
    return std::midpoint(low, high);
  } else {
    LINX_STATIC_ASSERT_FALSE("Unsupported parity");
  }

  // FIXME accept integral_constant?
}

/**
 * @brief Compute the median of an array of unknown size and size parity.
 */
KOKKOS_INLINE_FUNCTION auto median(auto& in_out)
{
  if (std::size(in_out) % 2 == 0) {
    return median<EvenNumber>(in_out);
  } else {
    return median<OddNumber>(in_out);
  }
}

/**
 * @brief Compute the median of an array of known size.
 */
template <std::integral auto N>
KOKKOS_INLINE_FUNCTION auto median(auto& in_out)
{
  return Impl::SelectNet<N>::median(in_out);
}

namespace Impl {

/**
 * @brief Fall back to parity-aware median if select net is not implemented for `N`.
 */
template <int N>
struct SelectNet {
  using Parity = std::conditional_t<(N % 2 == 0), EvenNumber, OddNumber>;
  KOKKOS_INLINE_FUNCTION static decltype(auto) median(auto& in_out)
  {
    return Linx::median<Parity>(in_out);
  }
};

} // namespace Impl
} // namespace Linx

#endif
