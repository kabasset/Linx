// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_SLICE_H
#define LINX_BASE_SLICE_H

#include "Linx/Base/Exceptions.h"
#include "Linx/Base/Functional.h" // Forward
#include "Linx/Base/Types.h"

#include <Kokkos_Core.hpp>
#include <concepts>
#include <ostream>

namespace Linx {

LINX_STRONG_TYPE(Size)

/**
 * @brief Functor which tests whether a value is between two inclusive or exclusive endpoints.
 */
template <bool InclusiveInfimum, bool InclusiveSupremum, typename T> // FIXME to Functional
class Between {
public:

  using value_type = T; ///< The value type

  /**
   * @brief Constructor.
   */
  Between(const T& infimum, const T& supremum) : m_infimum(infimum), m_supremum(supremum) {}

  /**
   * @brief Size-based constructor.
   */
  template <typename TSize>
  Between(const T&, const Size<TSize>& size) :
      m_infimum(infimum),
      m_supremum(m_infimum + size - std::is_integral_v<T>) {};

  /**
   * @brief Infimum.
   */
  KOKKOS_INLINE_FUNCTION value_type infimum() const
  {
    return m_infimum;
  }

  /**
   * @brief Supremum.
   */
  KOKKOS_INLINE_FUNCTION value_type supremum() const
  {
    return m_supremum;
  }

  /**
   * @brief Check whether a value is between the endpoints.
   */
  KOKKOS_INLINE_FUNCTION bool operator()(const T& value) const
  {
    return greater_than_infimum(value) && less_than_supremum(value);
  }

  /**
   * @brief Check whether a value is greater than the infimum.
   */
  KOKKOS_INLINE_FUNCTION bool greater_than_infimum(const T& value) const
  {
    if constexpr (InclusiveInfimum) {
      return value >= m_infimum;
    } else {
      return value > m_infimum;
    }
  }

  /**
   * @brief Check whether a value is less than the supremum.
   */
  KOKKOS_INLINE_FUNCTION bool less_than_supremum(const T& value) const
  {
    if constexpr (InclusiveSupremum) {
      return value <= m_supremum;
    } else {
      return value < m_supremum;
    }
  }

private:

  T m_infimum; ///< The interval infimum
  T m_supremum; ///< The interval supremum
};

template <typename T>
KOKKOS_INLINE_FUNCTION auto interval_size(const Between<true, false, T>& interval)
{
  return interval.m_supremum - interval.m_infimum + std::is_integral_v<T>;
}

template <typename T>
static constexpr auto interval_size(const Equal<Forward, T>&)
{
  return 1;
}

/// @cond

template <typename T, typename TFuncN, typename... TFuncs>
class Slice;

/// @endcond

namespace Impl {

/**
 * @brief Append a 1D slice.
 */
template <typename T, typename TFuncN, typename... TFuncs>
Slice<T, TFuncN, TFuncs...> slice_push_back(Slice<T, TFuncs...> slice, Slice<T, TFuncN> back)
{
  return Slice<T, TFuncN, TFuncs...>(LINX_MOVE(slice), LINX_MOVE(back));
}

/**
 * @brief Emplace a 1D slice.
 */
KOKKOS_INLINE_FUNCTION auto slice_emplace(auto slice, auto&&... args)
{
  return slice_push_back(LINX_MOVE(slice), Slice(LINX_FORWARD(args)...));
}

} // namespace Impl

/**
 * @brief Get the interval along i-th axis.
 */
template <int I, typename T, typename... TFuncs>
KOKKOS_INLINE_FUNCTION constexpr auto& get(const Slice<T, TFuncs...>& slice) // FIXME in std:: ?
{
  if constexpr (sizeof...(TFuncs) == 1) {
    return slice;
  } else {
    return slice.template get<I>();
  }
}

/**
 * @brief 1D interval.
 * @tparam TFunc The predicate defining the interval
 */
template <typename T, typename TFunc>
class Slice<T, TFunc> {
public:

  using size_type = T; ///< The index and size type

  /**
   * @brief Forwarding constructor.
   */
  Slice(std::convertible_to<size_type> auto&&... args) : m_func(LINX_FORWARD(args)...) {}

  /**
   * @brief Emplace another interval in a new axis.
   */
  auto operator()(auto&&... args) const
  {
    return Impl::slice_emplace(*this, LINX_FORWARD(args)...);
  }

  /**
   * @brief The region rank, always 1.
   */
  static constexpr int rank()
  {
    return 1;
  }

  KOKKOS_INLINE_FUNCTION auto size() const
  {
    return interval_size(m_func);
  }

  /**
   * @brief Predicate defining the interval.
   */
  KOKKOS_INLINE_FUNCTION const TFunc& func() const // FIXME better name?
  {
    return m_func;
  }

  /**
   * @brief Check if the slice contains a given value.
   */
  KOKKOS_INLINE_FUNCTION bool contains(const auto& value) const
  {
    return m_func(value);
  }

private:

  TFunc m_func; ///< The predicate defining the interval
};

/**
 * @brief Unbounded interval.
 */
template <typename T>
using Unbounded = Slice<T, std::bool_constant<true>>;

/**
 * @brief Singleton interval.
 */
template <typename T>
using Singleton = Slice<T, Equal<Forward, T>>;

/**
 * @brief Closed-open interval.
 */
template <typename T>
using Span = Slice<T, Between<true, false, T>>;

/**
 * @brief Closed interval.
 */
template <typename T>
using Segment = Slice<T, Between<true, true, T>>;

/**
 * @brief Deduction guide for unbounded slices.
 */
template <typename T = Index>
Slice()->Unbounded<T>;

/**
 * @brief Single value deduction guide for singletons.
 */
template <typename T>
Slice(const T&)->Singleton<T>;

/**
 * @brief Start and stop deduction guide for spans.
 */
template <typename T0, typename T1>
Slice(const T0&, const T1&)->Span<decltype(T1() - T0())>;

/**
 * @brief Start and size deduction guide for spans.
 */
template <typename T, typename TSize>
Slice(const T&, const Size<TSize>&)->Span<T>;

/**
 * @brief Copy deduction guide for any slice.
 */
template <typename T, typename TFunc>
Slice(const Slice<T, TFunc>&) -> Slice<T, TFunc>;

/**
 * @brief Stream insertion for an unbounded interval.
 */
template <std::integral T>
std::ostream& operator<<(std::ostream& os, const Unbounded<T>&)
{
  os << ':';
  return os;
}

/**
 * @brief Stream insertion for a span.
 */
template <std::integral T>
std::ostream& operator<<(std::ostream& os, const Span<T>& interval)
{
  os << interval.func().infimum() << ':' << interval.func().supremum();
  return os;
}

/**
 * @brief Stream insertion for a singleton.
 */
template <std::integral T>
std::ostream& operator<<(std::ostream& os, const Singleton<T>& interval)
{
  os << interval.func().rhs;
  return os;
}

template <std::integral T>
KOKKOS_INLINE_FUNCTION auto start(const Singleton<T>& slice)
{
  return slice.func().rhs;
}

template <std::integral T>
KOKKOS_INLINE_FUNCTION auto stop(const Singleton<T>& slice)
{
  return slice.func().rhs + 1;
}

template <std::integral T>
KOKKOS_INLINE_FUNCTION auto start(const Span<T>& slice)
{
  return slice.func().infimum();
}

template <std::integral T>
KOKKOS_INLINE_FUNCTION auto stop(const Span<T>& slice)
{
  return slice.func().supremum();
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
template <typename T, typename TFuncN, typename... TFuncs>
class Slice {
public:

  using size_type = T; ///< The index and size type
  static constexpr int n = sizeof...(TFuncs) + 1; ///< The dimension

  /**
   * @brief Extending constructor.
   * 
   * Prefer creating slices using the `operator()` syntax.
   */
  Slice(Slice<T, TFuncs...> fronts, Slice<T, TFuncN> back) : m_fronts(LINX_MOVE(fronts)), m_back(LINX_MOVE(back)) {}

  /**
   * @brief Extend the slice.
   */
  auto operator()(auto&&... args) const
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
   * The function is ill-formed if one of the intervals is infinite.
   */
  KOKKOS_INLINE_FUNCTION auto size() const
  {
    return m_fronts.size() * m_back.size();
  }

  /**
   * @brief Get the interval along i-th axis.
   */
  template <int I>
  KOKKOS_INLINE_FUNCTION constexpr auto& get() const
  {
    if constexpr (I == n - 1) {
      return m_back;
    } else {
      return Linx::get<I>(m_fronts);
    }
  }

  KOKKOS_INLINE_FUNCTION const auto& fronts() const
  {
    return m_fronts;
  }

  KOKKOS_INLINE_FUNCTION const auto& back() const
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

  Slice<T, TFuncs...> m_fronts; ///< The front slices
  Slice<T, TFuncN> m_back; ///< The back slice
};

/**
 * @relatesalso Slice
 * @brief Make a 1D slice clamped between bounds.
 */
template <typename T>
Span<T> clamp(const Unbounded<T>&, const auto& start, const auto& stop)
{
  return Span<T>(start, stop);
}

/**
 * @relatesalso Slice
 * @brief Make a 1D slice clamped between bounds.
 */
template <typename T>
Span<T> clamp(const Singleton<T>& interval, const auto& start, const auto& stop)
{
  const auto value = interval.func().rhs;
  if (Span(start, stop).contains(value)) {
    return Span<T>(value, value + 1);
  }
  return Span<T>(); // Empty
}

/**
 * @relatesalso Slice
 * @brief Make a 1D slice clamped between bounds.
 */
template <typename T>
Span<T> clamp(const Span<T>& interval, const auto& start, const auto& stop)
{
  return Span<T>(std::max<T>(interval.func().infimum(), start), std::min<T>(interval.func().supremum(), stop));
}

/**
 * @ingroup regions
 * @brief Apply a function to each element of the domain.
 * @tparam TSpace The execution space
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T, typename TInterval>
void for_each(const std::string& label, const Slice<T, TInterval>& slice, auto&& func)
{
  Kokkos::parallel_for(label, kokkos_execution_policy<TSpace>(slice), LINX_FORWARD(func));
}

} // namespace Linx

#endif
