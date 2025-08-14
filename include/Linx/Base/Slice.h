// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_SLICE_H
#define LINX_BASE_SLICE_H

#include "Linx/Base/Functional.h" // Forward
#include "Linx/Base/Interval.h"
#include "Linx/Base/Types.h"

#include <Kokkos_Core.hpp>
#include <concepts>
#include <ostream>

namespace Linx {

/**
 * @brief Get the interval along i-th axis.
 */
template <int I, typename T, typename... TPreds>
KOKKOS_INLINE_FUNCTION constexpr auto& get(const Slice<T, TPreds...>& slice)
{
  if constexpr (sizeof...(TPreds) == 1) {
    return slice;
  } else {
    return slice.template get<I>();
  }
}

/**
 * @ingroup regions
 * @brief ND slice.
 * 
 * A slice is an ND region made of intervals along successive axes.
 * Slices are built iteratively by calling `operator()`.
 * For example, Python's `[:, 10, 3:14]` writes `Slice()(10)(3, 14)`.
 * 
 * Slices are similar to bounding boxes, except that:
 * - slices can be unbounded;
 * - slices are defined axis-by-axis while boxes are defined by two ND positions.
 */
template <typename T, typename TPredN, typename... TPreds>
class Slice {
public:

  using size_type = T; ///< The index and size type
  static constexpr int n = sizeof...(TPreds) + 1; ///< The rank

  /**
   * @brief Extending constructor.
   */
  KOKKOS_INLINE_FUNCTION Slice(Forward, Slice<T, TPreds...> lower, Slice<T, TPredN> last) :
      m_lower(LINX_MOVE(lower)),
      m_last(LINX_MOVE(last))
  {}

  /**
   * @brief Extend the slice by emplacing an interval.
   */
  KOKKOS_INLINE_FUNCTION auto operator()(auto&&... args) const
  {
    return Impl::slice_emplace(*this, LINX_FORWARD(args)...);
  }

  /**
   * @brief Region rank.
   */
  static constexpr int rank()
  {
    return n;
  }

  /**
   * @brief Region size.
   * 
   * The function is ill-formed if any of the intervals is infinite.
   */
  KOKKOS_INLINE_FUNCTION auto size() const
  {
    return m_lower.size() * m_last.size();
  }

  /**
   * @brief Start index along i-th axis.
   */
  KOKKOS_INLINE_FUNCTION auto start(std::integral auto i) const
  {
    if (i == n - 1) {
      return m_last.start();
    } else {
      return m_lower.start(i);
    }
  }

  /**
   * @brief Stop index along i-th axis.
   */
  KOKKOS_INLINE_FUNCTION auto stop(std::integral auto i) const
  {
    if (i == n - 1) {
      return m_last.stop();
    } else {
      return m_lower.stop(i);
    }
  }

  /**
   * @brief Interval along i-th axis.
   */
  template <int I>
  KOKKOS_INLINE_FUNCTION constexpr auto& get() const
  {
    if constexpr (I == n - 1) {
      return m_last;
    } else {
      return Linx::get<I>(m_lower);
    }
  }

  /**
   * @brief Slice of immediately lower rank.
   */
  KOKKOS_INLINE_FUNCTION const auto& lower() const
  {
    return m_lower;
  }

  /**
   * @brief Interval along last axis.
   */
  KOKKOS_INLINE_FUNCTION const auto& last() const
  {
    return m_last;
  }

  /**
   * @brief Stream insertion, following Python's syntax.
   * 
   * For example:
   * 
   * \code
   * std::cout << Slice(10)()(3, 14) << std::endl;
   * \endcode
   * 
   * prints `10, :, 3:14`.
   */
  friend std::ostream& operator<<(std::ostream& os, const Slice& slice)
  {
    os << slice.m_lower << ", " << slice.m_last;
    return os;
  }

private:

  Slice<T, TPreds...> m_lower; ///< The lower-rank slice
  Slice<T, TPredN> m_last; ///< The last slice
};

} // namespace Linx

#endif
