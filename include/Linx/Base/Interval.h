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

template <typename T, typename TPredN, typename... TPreds>
class Slice;

/// @endcond

namespace Impl {

/**
 * @brief Helper function to benefit from CTAD on NVCC.
 */
KOKKOS_INLINE_FUNCTION auto slice_emplace(const auto& slice, auto&&... args)
{
  return Slice(Forward(), slice, Slice(LINX_FORWARD(args)...));
}

} // namespace Impl

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
 * @brief Interval (slice of rank 1).
 * @tparam TPred The predicate defining the interval
 */
template <typename T, typename TPred>
class Slice<T, TPred> {
public:

  using size_type = T; ///< The index and size type
  using Pred = TPred; ///< The predicate defining the interval
  static constexpr auto n = 1; ///< The region rank

  /**
   * @brief Forwarding constructor.
   */
  KOKKOS_INLINE_FUNCTION Slice(auto&&... args) : m_pred(LINX_FORWARD(args)...) {}

  /**
   * @brief Emplace another interval in a new axis.
   */
  KOKKOS_INLINE_FUNCTION auto operator()(auto&&... args) const
  {
    return Impl::slice_emplace(*this, LINX_FORWARD(args)...);
  }

  /**
   * @brief Region rank, always 1.
   */
  static constexpr int rank()
  {
    return 1;
  }

  /**
   * @brief Interval size, if bounded.
   * 
   * The function is ill-formed if `interval_size(pred())` is not defined.
   */
  KOKKOS_INLINE_FUNCTION auto size() const
  {
    return interval_size(m_pred);
  }

  /**
   * @brief Start index, if it exists.
   * 
   * The function is ill-formed if `interval_start(pred())` is not defined.
   */
  KOKKOS_INLINE_FUNCTION auto start() const
  {
    return interval_start(m_pred);
  }

  /**
   * @brief Stop index, if it exists.
   * 
   * The function is ill-formed if `interval_stop(pred())` is not defined.
   */
  KOKKOS_INLINE_FUNCTION auto stop() const
  {
    return interval_stop(m_pred);
  }

  /**
   * @brief Predicate defining the interval.
   */
  KOKKOS_INLINE_FUNCTION const Pred& pred() const // FIXME better name?
  {
    return m_pred;
  }

  /**
   * @brief Check if the slice contains a given value.
   */
  KOKKOS_INLINE_FUNCTION bool contains(const auto& value) const
  {
    return m_pred(value);
  }

private:

  Pred m_pred; ///< The predicate defining the interval
};

/**
 * @brief Exception thrown if a value lies out of given bounds.
 * 
 * @tparam TSlice The interval type
 */
template <typename TSlice>
class OutOfBounds : public Exception {
public:

  OutOfBounds(const std::string& name, auto value, const TSlice& bounds) :
      Exception("Out of bounds", name + " " + std::to_string(value) + " not in ")
  {
    std::stringstream ss;
    ss << bounds;
    append(ss.str());
  }

  /**
   * @brief Constructor.
   */
  OutOfBounds(const std::string& name, auto value, const auto&... args) : OutOfBounds(name, value, TSlice(args...)) {}

  /**
   * @brief Throw if a value lies out of given bounds.
   */
  static void may_throw(const std::string& name, auto value, auto&&... args)
  {
    if (not TSlice(LINX_FORWARD(args)...).contains(value)) {
      throw OutOfBounds(name, value, LINX_FORWARD(args)...);
    }
  }
};

/**
 * @relatesalso Slice
 * @brief Make an interval clamped between bounds.
 */
template <std::integral T>
KOKKOS_INLINE_FUNCTION Span<T> clamp(const Unbounded<T>&, const auto& start, const auto& stop)
{
  return Span<T>(start, stop);
}

/**
 * @relatesalso Slice
 * @brief Make an interval clamped between bounds.
 */
template <std::integral T>
KOKKOS_INLINE_FUNCTION Singleton<T> clamp(const Singleton<T>& interval, const auto& start, const auto& stop)
{
  OutOfBounds<Span<T>>::may_throw("singleton", interval.start(), start, stop);
  // Always return a singleton, not a span, to ensure slicing with a singleton reduces rank
  return interval;
}

/**
 * @relatesalso Slice
 * @brief Make an interval clamped between bounds.
 */
template <std::integral T>
KOKKOS_INLINE_FUNCTION Span<T> clamp(const Span<T>& interval, const auto& start, const auto& stop)
{
  return Span<T>(std::max<T>(interval.pred().infimum, start), std::min<T>(interval.pred().supremum, stop));
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
