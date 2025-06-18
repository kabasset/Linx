// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_SLICE_H
#define LINX_BASE_SLICE_H

#include "Linx/Base/Exceptions.h"
#include "Linx/Base/Interval.h"
#include "Linx/Base/Types.h"

#include <Kokkos_Core.hpp>
#include <concepts>
#include <ostream>

namespace Linx {

/// @cond

template <typename T, Interval TIntervalN, Interval... TIntervals>
class Slice;

/// @endcond

namespace Impl {

/**
 * @brief Append a 1D slice.
 */
template <typename T, Interval TIntervalN, Interval... TIntervals>
Slice<T, TIntervalN, TIntervals...> slice_push_back(Slice<T, TIntervals...> slice, TIntervalN back)
{
  return Slice<T, TIntervalN, TIntervals...>(slice, LINX_MOVE(back));
}

/**
 * @brief Emplace a 1D slice.
 */
KOKKOS_INLINE_FUNCTION auto slice_emplace(auto slice, auto&&... args)
{
  return slice_push_back(LINX_MOVE(slice), Slice(LINX_FORWARD(args)...).template get<0>());
}

} // namespace Impl

/**
 * @brief 1D specialization.
 */
template <typename T, typename TInterval>
class Slice<T, TInterval> {
public:

  using size_type = T; ///< The value type of the interval
  static constexpr int n = 1; ///< The rank

  /**
   * @brief Implicit conversion constructor.
   */
  Slice(TInterval interval) : m_back(LINX_MOVE(interval)) {}

  /**
   * @brief Constructor.
   */
  Slice(std::convertible_to<T> auto&&... args) : m_back(LINX_FORWARD(args)...) {}

  auto operator()(auto&&... args) const
  {
    return Impl::slice_emplace(*this, LINX_FORWARD(args)...);
  }

  template <int I>
  KOKKOS_INLINE_FUNCTION constexpr auto& get() const
  {
    static_assert(I == 0);
    return m_back;
  }

  friend std::ostream& operator<<(std::ostream& os, const Slice& slice)
  {
    os << slice.m_back;
    return os;
  }

private:

  TInterval m_back; ///< The back slice
};

template <typename T = Index> // Required by some compilers such as ICX
Slice() -> Slice<T, Unbounded>;

template <typename T>
Slice(const T&) -> Slice<T, Singleton<T>>;

template <typename T0, typename T1>
Slice(const T0&, const T1&) -> Slice<
    decltype(std::declval<T1>() - std::declval<T0>()),
    Span<decltype(std::declval<T1>() - std::declval<T0>())>>;

template <typename T, typename TSize>
Slice(const T&, const Size<TSize>&) -> Slice<T, Span<T>>;

/**
 * @brief Get the interval along i-th axis.
 */
template <int I, typename T, Interval... TIntervals>
KOKKOS_INLINE_FUNCTION constexpr auto& get(const Slice<T, TIntervals...>& slice) // FIXME in std:: ?
{
  return slice.template get<I>();
}

/**
 * @ingroup regions
 * @brief ND slice.
 * 
 * A slice is an ND sequence made of intervals along successive axes.
 * Slices are built iteratively by calling `operator()`.
 * For example, Python's `[:, 10, 3:14]` writes `Slice()(10)(3, 14)`.
 * 
 * Slices are similar to bounding boxes, except that:
 * - slices can be unbounded;
 * - slices are defined axis-by-axis while boxes are defined by two ND positions.
 */
template <typename T, Interval TIntervalN, Interval... TIntervals>
class Slice {
public:

  using size_type = T; ///< The value type of the intervals
  static constexpr int n = sizeof...(TIntervals) + 1; ///< The dimension

  /**
   * @brief Constructor.
   * 
   * Prefer creating slices using the `operator()` syntax.
   */
  Slice(Slice<T, TIntervals...> fronts, auto&&... args) : m_fronts(LINX_MOVE(fronts)), m_back(LINX_FORWARD(args)...) {}

  /**
   * @brief Extend the slice.
   */
  auto operator()(auto&&... args) const
  {
    return Impl::slice_emplace(*this, LINX_FORWARD(args)...);
  }

  /**
   * @brief Get the 1D slice along i-th axis.
   */
  template <int I>
  KOKKOS_INLINE_FUNCTION constexpr auto& get() const
  {
    if constexpr (I == n - 1) {
      return m_back;
    } else {
      return m_fronts.template get<I>(); // FIXME std::get<I>(m_fronts) ?
    }
  }

  [[deprecated]] KOKKOS_INLINE_FUNCTION const auto& fronts() const
  {
    return m_fronts;
  }

  [[deprecated]] KOKKOS_INLINE_FUNCTION const auto& back() const
  {
    return m_back;
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
    os << slice.m_fronts << ", " << slice.m_back;
    return os;
  }

private:

  Slice<T, TIntervals...> m_fronts; ///< The front slices
  TIntervalN m_back; ///< The back slice
};

/**
 * @relatesalso Slice
 * @brief Make a 1D slice clamped between bounds.
 */
template <typename T>
Slice<T, Span<T>> clamp(const Unbounded&, const T& start, const T& stop)
{
  return {start, stop};
}

/**
 * @relatesalso Slice
 * @brief Make a 1D slice clamped between bounds.
 */
template <typename T>
Slice<T, Singleton<T>> clamp(const Singleton<T>& interval, const auto& start, const auto& stop)
{
  OutOfBounds<'[', ')'>::may_throw("slice index", interval.value(), {start, stop}); // FIXME based on Span
  return {interval.value()};
}

/**
 * @relatesalso Slice
 * @brief Make a 1D slice clamped between bounds.
 */
template <typename T>
Slice<T, Span<T>> clamp(const Span<T>& interval, const auto& start, const auto& stop)
{
  return {std::max<T>(interval.start(), start), std::min<T>(interval.stop(), stop)};
}

} // namespace Linx

#endif
