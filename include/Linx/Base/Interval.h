// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_INTERVAL_H
#define LINX_BASE_INTERVAL_H

#include "Linx/Base/Exceptions.h"
#include "Linx/Base/Types.h"

#include <Kokkos_Core.hpp>
#include <concepts>
#include <ostream>

namespace Linx {

LINX_STRONG_TYPE(Size)

/**
 * @ingroup regions
 * @brief Singleton.
 */
template <typename T>
class Singleton {
public:

  using value_type = T; ///< The value type

  /**
   * @brief Constructor.
   */
  KOKKOS_INLINE_FUNCTION Singleton(T value) : m_value(value) {}

  /**
   * @brief Single value.
   */
  KOKKOS_INLINE_FUNCTION T value() const
  {
    return m_value;
  }

  /**
   * @brief Check if the interval contains a value.
   */
  KOKKOS_INLINE_FUNCTION bool contains(const auto& value) const
  {
    return value == m_value;
  }

  /**
   * @brief Stream the interval.
   */
  friend std::ostream& operator<<(std::ostream& os, const Singleton& interval)
  {
    os << interval.m_value;
    return os;
  }

private:

  T m_value; ///< The single value
};

/**
 * @ingroup regions
 * @brief Closed-open interval.
 */
template <typename T>
class Span {
public:

  using value_type = T; ///< The value type

  /**
   * @brief Constructor.
   */
  KOKKOS_INLINE_FUNCTION Span(const T& start, const T& stop) : m_start(start), m_stop(stop) {}

  /**
   * @brief Size-based constructor.
   */
  template <typename U>
  KOKKOS_INLINE_FUNCTION Span(const T& start, const Size<U>& size) : m_start(start), m_stop(m_start + size.value)
  {}

  /**
   * @brief Start endpoint (inclusive).
   */
  KOKKOS_INLINE_FUNCTION T start() const
  {
    return m_start;
  }

  /**
   * @brief Stop endpoint (exclusive).
   */
  KOKKOS_INLINE_FUNCTION T stop() const
  {
    return m_stop;
  }

  /**
   * @brief Interval size.
   */
  KOKKOS_INLINE_FUNCTION T size() const
  {
    return m_stop - m_start;
  }

  /**
   * @brief Check if the interval contains a value.
   */
  KOKKOS_INLINE_FUNCTION bool contains(const T& value) const
  {
    return value >= m_start && value < m_stop;
  }

  /**
   * @brief Stream the interval.
   */
  friend std::ostream& operator<<(std::ostream& os, const Span& interval)
  {
    os << interval.m_start << ':' << interval.m_stop;
    return os;
  }

private:

  T m_start; ///< Start endpoint (inclusive)
  T m_stop; ///< Stop endpoint (exclusive)
};

template <typename T0, typename T1>
Span(T0, T1) -> Span<decltype(std::declval<T1>() - std::declval<T0>())>;

/**
 * @ingroup regions
 * @brief Closed interval.
 */
template <typename T>
class Segment {
public:

  using value_type = T; ///< The value type

  /**
   * @brief Constructor.
   */
  KOKKOS_INLINE_FUNCTION Segment(const T& start, const T& finish) : m_start(start), m_finish(finish) {}

  /**
   * @brief Size-based constructor.
   */
  template <typename U>
  KOKKOS_INLINE_FUNCTION Segment(const T& start, const Size<U>& size) :
      m_start(start),
      m_finish(m_start + size.value - std::is_integral_v<T>)
  {}

  /**
   * @brief Start endpoint (inclusive).
   */
  KOKKOS_INLINE_FUNCTION T start() const
  {
    return m_start;
  }

  /**
   * @brief Finish endpoint (inclusive).
   */
  KOKKOS_INLINE_FUNCTION T finish() const
  {
    return m_finish;
  }

  /**
   * @brief Interval size.
   */
  KOKKOS_INLINE_FUNCTION T size() const
  {
    return m_finish - m_start + std::is_integral_v<T>;
  }

  /**
   * @brief Check if the interval contains a value.
   */
  KOKKOS_INLINE_FUNCTION bool contains(const T& value) const
  {
    return value >= m_start && value <= m_finish;
  }

private:

  T m_start;
  T m_finish;
};

template <typename T0, typename T1>
Segment(T0, T1) -> Segment<decltype(std::declval<T1>() - std::declval<T0>())>;

/**
 * @ingroup regions
 * @brief Closed-infinite interval.
 */
template <typename T>
class LowerBound {
public:

  using value_type = T; ///< The value type

  /**
   * @brief Constructor.
   */
  KOKKOS_INLINE_FUNCTION LowerBound(const T& start) : m_start(start) {}

  /**
   * @brief Start endpoint (inclusive).
   */
  KOKKOS_INLINE_FUNCTION T start() const
  {
    return m_start;
  }

  /**
   * @brief Check if the interval contains a value.
   */
  KOKKOS_INLINE_FUNCTION bool contains(const T& value) const
  {
    return value >= m_start;
  }

  /**
   * @brief Stream the interval.
   */
  friend std::ostream& operator<<(std::ostream& os, const LowerBound& interval)
  {
    os << interval.m_start << ':';
    return os;
  }

private:

  T m_start;
};

/**
 * @ingroup regions
 * @brief Unbounded region.
 */
class Unbounded {
public:

  /**
   * @brief Check if the interval contains a value. Always true.
   */
  KOKKOS_INLINE_FUNCTION static constexpr bool contains(auto&&...)
  {
    return true;
  }

  /**
   * @brief Stream the interval.
   */
  friend std::ostream& operator<<(std::ostream& os, const Unbounded&)
  {
    os << ':';
    return os;
  }
};

namespace Impl {

template <typename T>
struct IntervalTraits {
  static constexpr bool is_interval = false;
};

template <typename T>
struct IntervalTraits<Singleton<T>> {
  static constexpr bool is_interval = true;
  static constexpr bool is_bounded = true;
  static constexpr bool is_kokkos_slice = true;
};

template <typename T>
struct IntervalTraits<Span<T>> {
  static constexpr bool is_interval = true;
  static constexpr bool is_bounded = true;
  static constexpr bool is_kokkos_slice = true;
};

template <typename T>
struct IntervalTraits<Segment<T>> {
  static constexpr bool is_interval = true;
  static constexpr bool is_bounded = true;
  static constexpr bool is_kokkos_slice = true;
};

template <>
struct IntervalTraits<Unbounded> {
  static constexpr bool is_interval = true;
  static constexpr bool is_bounded = false;
  static constexpr bool is_kokkos_slice = true;
};

} // namespace Impl

template <typename T>
concept Interval = Impl::IntervalTraits<T>::is_interval;

template <typename T>
concept BoundedInterval = Interval<T> && Impl::IntervalTraits<T>::is_bounded;

template <typename T>
concept KokkosSlice = Impl::IntervalTraits<T>::is_kokkos_slice; // FIXME detect(kokkos_slice(T()))

/**
 * @brief Make a Kokkos slice from an integral singleton.
 */
template <std::integral T>
KOKKOS_INLINE_FUNCTION auto kokkos_slice(const Singleton<T>& interval)
{
  return interval.value();
}

/**
 * @brief Make a Kokkos slice from an integral span.
 */
template <std::integral T>
KOKKOS_INLINE_FUNCTION auto kokkos_slice(const Span<T>& interval)
{
  return Kokkos::pair(interval.start(), interval.stop());
}

/**
 * @brief Make a Kokkos slice from an integral segment.
 */
template <std::integral T>
KOKKOS_INLINE_FUNCTION auto kokkos_slice(const Segment<T>& interval)
{
  return Kokkos::pair(interval.start(), interval.finish() + 1);
}

/**
 * @brief Make an unbounded Kokkos slice.
 */
KOKKOS_INLINE_FUNCTION auto kokkos_slice(const Unbounded&)
{
  return Kokkos::ALL;
}

/**
 * @brief Get the Kokkos execution policy of a span.
 */
template <typename TSpace, std::integral T>
auto kokkos_execution_policy(const Singleton<T>& interval)
{
  return Kokkos::RangePolicy<TSpace, Kokkos::IndexType<Index>>(interval.value(), interval.value() + 1);
}

/**
 * @brief Get the Kokkos execution policy of a span.
 */
template <typename TSpace, std::integral T>
auto kokkos_execution_policy(const Span<T>& interval)
{
  return Kokkos::RangePolicy<TSpace, Kokkos::IndexType<Index>>(interval.start(), interval.stop());
}

/**
 * @brief Get the Kokkos execution policy of a span.
 */
template <typename TSpace, std::integral T>
auto kokkos_execution_policy(const Segment<T>& interval)
{
  return Kokkos::RangePolicy<TSpace, Kokkos::IndexType<Index>>(interval.start(), interval.finish() + 1);
}

/**
 * @ingroup regions
 * @brief Apply a function to each element of the domain.
 * @tparam TSpace The execution space
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
void for_each(const std::string& label, const KokkosSlice auto& interval, auto&& func)
{
  Kokkos::parallel_for(label, kokkos_execution_policy<TSpace>(interval), LINX_FORWARD(func));
}

} // namespace Linx

#endif
