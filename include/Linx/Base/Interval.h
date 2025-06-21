// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_INTERVAL_H
#define LINX_BASE_INTERVAL_H

#include "Linx/Base/Exceptions.h"
#include "Linx/Base/Functional.h"
#include "Linx/Base/Types.h"

#include <concepts>

namespace Linx {

/**
 * @brief Size of a bounded interval.
 */
template <bool InclusiveInfimum, bool InclusiveSupremum, typename T>
KOKKOS_INLINE_FUNCTION auto interval_size(const Between<InclusiveInfimum, InclusiveSupremum, T>& interval)
{
  if constexpr (std::is_integral_v<T>) {
    return interval.supremum - interval.infimum + InclusiveInfimum + InclusiveSupremum - 1;
  } else {
    return interval.supremum - interval.infimum;
  }
}

/**
 * @brief Start index of an integral bounded interval.
 */
template <bool InclusiveInfimum, bool InclusiveSupremum, std::integral T>
KOKKOS_INLINE_FUNCTION auto interval_start(const Between<InclusiveInfimum, InclusiveSupremum, T>& interval)
{
  return interval.infimum + (not InclusiveInfimum);
}

/**
 * @brief Stop index of an integral bounded interval.
 */
template <bool InclusiveInfimum, bool InclusiveSupremum, std::integral T>
KOKKOS_INLINE_FUNCTION auto interval_stop(const Between<InclusiveInfimum, InclusiveSupremum, T>& interval)
{
  return interval.supremum + InclusiveSupremum;
}

/**
 * @brief Size of a singleton.
 */
template <typename T>
static constexpr auto interval_size(const Equal<Forward, T>&)
{
  return std::is_integral_v<T> ? 1 : 0;
}

/**
 * @brief Start index of a singleton.
 */
template <std::integral T>
KOKKOS_INLINE_FUNCTION auto interval_start(const Equal<Forward, T>& interval)
{
  return interval.rhs;
}

/**
 * @brief Stop index of a singleton.
 */
template <std::integral T>
KOKKOS_INLINE_FUNCTION auto interval_stop(const Equal<Forward, T>& interval)
{
  return interval.rhs + 1;
}

/// @cond

template <typename T, typename TFuncN, typename... TFuncs>
class Slice;

/// @endcond

/**
 * @brief Unbounded interval.
 */
template <typename T>
  requires(std::is_arithmetic_v<T>)
using Unbounded = Slice<T, StaticConstant<true>>;

/**
 * @brief Singleton interval.
 */
template <typename T>
  requires(std::is_arithmetic_v<T>)
using Singleton = Slice<T, Equal<Forward, T>>;

/**
 * @brief Closed-open interval.
 */
template <typename T>
  requires(std::is_arithmetic_v<T>)
using Span = Slice<T, Between<true, false, T>>;

/**
 * @brief Closed interval.
 */
template <typename T>
  requires(std::is_arithmetic_v<T>)
using Segment = Slice<T, Between<true, true, T>>;

/**
 * @brief Deduction guide for unbounded slices.
 */
template <typename T = Index>
  requires(std::is_arithmetic_v<T>)
Slice()->Unbounded<T>;

/**
 * @brief Single value deduction guide for singletons.
 */
template <typename T>
  requires(std::is_arithmetic_v<T>)
Slice(const T&)->Singleton<T>;

/**
 * @brief Start and stop deduction guide for spans.
 */
template <typename T0, typename T1>
  requires(std::is_arithmetic_v<T0> && std::is_arithmetic_v<T1>)
Slice(const T0&, const T1&)->Span<decltype(T1() - T0())>;

/**
 * @brief Start and size deduction guide for spans.
 */
template <typename T, typename TSize>
  requires(std::is_arithmetic_v<T>)
Slice(const T&, const Size<TSize>&)->Span<T>;

/**
 * @brief 1D interval.
 * @tparam TFunc The predicate defining the interval
 */
template <typename T, typename TFunc>
class Slice<T, TFunc> {
public:

  using size_type = T; ///< The index and size type
  using Func = TFunc; ///< The predicate defining the interval
  static constexpr auto n = 1; ///< The region rank

  // Copy and move ctors are needed because of the forwarding ctor below.
  LINX_DEFAULT_COPYABLE(Slice);
  LINX_DEFAULT_MOVABLE(Slice);
  ~Slice() = default;

  /**
   * @brief Forwarding constructor.
   */
  Slice(auto&&... args) : m_func(LINX_FORWARD(args)...) {}

  /**
   * @brief Emplace another interval in a new axis.
   */
  auto operator()(auto&&... args) const
  {
    return Linx::Slice(Forward(), *this, Linx::Slice(LINX_FORWARD(args)...));
  }

  /**
   * @brief The region rank, always 1.
   */
  static constexpr int rank()
  {
    return 1;
  }

  /**
   * @brief The interval size, if bounded.
   * 
   * The function is ill-formed if `interval_size(func())` is not defined.
   */
  KOKKOS_INLINE_FUNCTION auto size() const
  {
    return interval_size(m_func);
  }

  /**
   * @brief Start index, if it exists.
   * 
   * The function is ill-formed if `interval_start(func())` is not defined.
   */
  KOKKOS_INLINE_FUNCTION auto start() const
  {
    return interval_start(m_func);
  }

  /**
   * @brief Stop index, if it exists.
   * 
   * The function is ill-formed if `interval_stop(func())` is not defined.
   */
  KOKKOS_INLINE_FUNCTION auto stop() const
  {
    return interval_stop(m_func);
  }

  /**
   * @brief Predicate defining the interval.
   */
  KOKKOS_INLINE_FUNCTION const Func& func() const // FIXME better name?
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

  Func m_func; ///< The predicate defining the interval
};

/**
 * @relatesalso Slice
 * @brief Make an interval clamped between bounds.
 */
template <std::integral T>
Span<T> clamp(const Unbounded<T>&, const auto& start, const auto& stop)
{
  return Span<T>(start, stop);
}

/**
 * @relatesalso Slice
 * @brief Make an interval clamped between bounds.
 */
template <std::integral T>
Singleton<T> clamp(const Singleton<T>& interval, const auto& start, const auto& stop)
{
  // FIXME OutOfBounds<Span<T>>::may_throw("singleton", interval.start(), Slice(start, stop));
  OutOfBounds<'[', ')'>::may_throw("singleton", interval.start(), {start, stop});
  // Return an interval, not a span, to ensure slicing with a singleton reduces rank
  return interval;
}

/**
 * @relatesalso Slice
 * @brief Make an interval clamped between bounds.
 */
template <std::integral T>
Span<T> clamp(const Span<T>& interval, std::convertible_to<T> auto start, std::convertible_to<T> auto stop)
{
  return Span<T>(std::max<T>(interval.func().infimum, start), std::min<T>(interval.func().supremum, stop));
}

/**
 * @brief Kokkos slicing argument of an unbounded interval.
 */
template <std::integral T>
KOKKOS_INLINE_FUNCTION auto kokkos_slice(const Unbounded<T>&)
{
  return Kokkos::ALL;
}

/**
 * @brief Kokkos slicing argument of a singleton.
 */
template <std::integral T>
KOKKOS_INLINE_FUNCTION auto kokkos_slice(const Singleton<T>& interval)
{
  return interval.start();
}

/**
 * @brief Kokkos slicing argument of a span.
 */
template <std::integral T>
KOKKOS_INLINE_FUNCTION auto kokkos_slice(const Span<T>& interval)
{
  return Kokkos::pair(interval.start(), interval.stop());
}

/**
 * @brief Kokkos slicing argument of a segment.
 */
template <std::integral T>
KOKKOS_INLINE_FUNCTION auto kokkos_slice(const Segment<T>& interval)
{
  return Kokkos::pair(interval.start(), interval.stop());
}

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
 * @brief Stream insertion for a singleton.
 */
template <std::integral T>
std::ostream& operator<<(std::ostream& os, const Singleton<T>& interval)
{
  os << interval.start();
  return os;
}

/**
 * @brief Stream insertion for a span.
 */
template <std::integral T>
std::ostream& operator<<(std::ostream& os, const Span<T>& interval)
{
  os << interval.start() << ':' << interval.stop();
  return os;
}

/**
 * @brief Stream insertion for a segment.
 */
template <std::integral T>
std::ostream& operator<<(std::ostream& os, const Segment<T>& interval)
{
  os << interval.start() << ':' << interval.stop();
  return os;
}

} // namespace Linx

#endif
