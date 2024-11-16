// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_SLICE_H
#define LINX_BASE_SLICE_H

#include "Linx/Base/Exceptions.h"
#include "Linx/Base/Types.h"

#include <Kokkos_Core.hpp>
#include <concepts>
#include <ostream>

namespace Linx {

/**
 * @brief Type of 1D slice.
 */
enum class SliceType : char {
  unbounded = '*', ///< Unbounded
  singleton = '=', ///< Single value
  closed = ']', ///< Closed interval
  right_open = ')', ///< Right-open interval, a.k.a. span
  right_infinite = '+' ///< Right-infinite interval
};

LINX_STRONG_TYPE(Size)

/// @cond

template <typename T, SliceType TypeN, SliceType... Types>
class Slice;

/// @endcond

template <typename T = Index> // Required by some compilers such as ICX
Slice()->Slice<T, SliceType::unbounded>;

template <typename T>
Slice(const T&) -> Slice<T, SliceType::singleton>;

template <typename T>
Slice(const T&, const T&) -> Slice<T, SliceType::right_open>;

template <typename T, typename U>
Slice(const T&, const Size<U>&) -> Slice<T, SliceType::right_open>;

template <typename T>
Slice(const T& start, std::nullptr_t) -> Slice<T, SliceType::right_infinite>;

/**
 * @brief Get the slice along i-th axis.
 */
template <int I, typename T, SliceType... Types>
KOKKOS_INLINE_FUNCTION constexpr auto& get(const Slice<T, Types...>& slice)
{
  if constexpr (sizeof...(Types) == 1) {
    return slice;
  } else {
    return slice.template get<I>();
  }
}

/**
 * @brief Append a 1D slice.
 */
template <typename T, SliceType TypeN, SliceType... Types>
KOKKOS_INLINE_FUNCTION Slice<T, TypeN, Types...> slice_push_back(Slice<T, Types...> slice, Slice<T, TypeN> back)
{
  return Slice<T, TypeN, Types...>(slice, back);
}

/**
 * @brief Emplace a 1D slice.
 */
template <typename T, SliceType... Types>
KOKKOS_INLINE_FUNCTION auto slice_emplace(Slice<T, Types...> slice, auto... args)
{
  return slice_push_back(slice, Slice(args...));
}

/**
 * @brief ND slice.
 * 
 * Slices are built iteratively by calling `operator()`.
 * For example, Python's `[:, 10, 3:14]` writes `Slice()(10)(3, 14)`.
 * 
 * Slices are similar to bounding boxes, except that:
 * - slices can be unbounded;
 * - slices are defined axis-by-axis while boxes are defined by two ND positions.
 */
template <typename T, SliceType TypeN, SliceType... Types>
class Slice {
public:

  using size_type = T; ///< The value type
  static constexpr int n = sizeof...(Types) + 1; ///< The dimension

  /**
   * @brief Constructor.
   * 
   * Prefer creating slices using the `operator()` syntax.
   */
  KOKKOS_INLINE_FUNCTION Slice(Slice<T, Types...> fronts, Slice<T, TypeN> back) :
      m_fronts(LINX_MOVE(fronts)),
      m_back(LINX_MOVE(back))
  {}

  /**
   * @brief Extend the slice.
   */
  KOKKOS_INLINE_FUNCTION auto operator()(auto... args) const&
  {
    return slice_emplace(*this, args...);
  }

  /**
   * @brief Extend the slice.
   */
  KOKKOS_INLINE_FUNCTION auto operator()(auto... args) &&
  {
    return slice_emplace(LINX_MOVE(*this), args...);
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
   * @brief Get the 1D slice along i-th axis.
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

  Slice<T, Types...> m_fronts; ///< The front slices
  Slice<T, TypeN> m_back; ///< The back slice
};

/**
 * @brief 1D unbounded specialization.
 */
template <typename T>
class Slice<T, SliceType::unbounded> {
public:

  using size_type = T;
  static constexpr int n = 1;
  static constexpr SliceType type = SliceType::unbounded; // FIXME type?

  KOKKOS_INLINE_FUNCTION Slice() {}

  KOKKOS_INLINE_FUNCTION auto operator()(auto... args) const&
  {
    return slice_emplace(*this, args...);
  }

  KOKKOS_INLINE_FUNCTION auto operator()(auto... args) &&
  {
    return slice_emplace(LINX_MOVE(*this), args...);
  }

  KOKKOS_INLINE_FUNCTION static constexpr bool contains(const T&)
  {
    return true;
  }

  KOKKOS_INLINE_FUNCTION auto kokkos_slice() const // TODO free function
  {
    return Kokkos::ALL;
  }

  friend std::ostream& operator<<(std::ostream& os, const Slice&)
  {
    os << ':';
    return os;
  }
};

/**
 * @brief 1D singleton specialization.
 */
template <typename T>
class Slice<T, SliceType::singleton> {
public:

  using size_type = T;
  static constexpr int n = 1;
  static constexpr SliceType type = SliceType::singleton;

  KOKKOS_INLINE_FUNCTION Slice(T value) : m_value(value) {}

  KOKKOS_INLINE_FUNCTION auto operator()(auto... args) const&
  {
    return slice_emplace(*this, args...);
  }

  KOKKOS_INLINE_FUNCTION auto operator()(auto... args) &&
  {
    return slice_emplace(LINX_MOVE(*this), args...);
  }

  KOKKOS_INLINE_FUNCTION T value() const
  {
    return m_value;
  }

  KOKKOS_INLINE_FUNCTION bool contains(const T& value) const
  {
    return value == m_value;
  }

  KOKKOS_INLINE_FUNCTION auto kokkos_slice() const // TODO free function
  {
    return m_value;
  }

  friend std::ostream& operator<<(std::ostream& os, const Slice& slice)
  {
    os << slice.m_value;
    return os;
  }

private:

  T m_value;
};

/**
 * @brief 1D span specialization.
 */
template <typename T>
class Slice<T, SliceType::right_open> {
public:

  using size_type = T;
  static constexpr int n = 1;
  static constexpr SliceType type = SliceType::right_open;

  KOKKOS_INLINE_FUNCTION Slice(const T& start, const T& stop) : m_start(start), m_stop(stop) {}

  template <typename U>
  KOKKOS_INLINE_FUNCTION Slice(const T& start, const Size<U>& size) : m_start(start), m_stop(m_start + size.value)
  {}

  KOKKOS_INLINE_FUNCTION Slice(const T& start, std::nullptr_t) : m_start(start), m_stop(Limits<T>::inf()) {}

  KOKKOS_INLINE_FUNCTION auto operator()(auto... args) const&
  {
    return slice_emplace(*this, args...);
  }

  KOKKOS_INLINE_FUNCTION auto operator()(auto... args) &&
  {
    return slice_emplace(LINX_MOVE(*this), args...);
  }

  KOKKOS_INLINE_FUNCTION T start() const
  {
    return m_start;
  }

  KOKKOS_INLINE_FUNCTION T stop() const
  {
    return m_stop;
  }

  KOKKOS_INLINE_FUNCTION T size() const
  {
    return m_stop - m_start;
  }

  KOKKOS_INLINE_FUNCTION bool contains(const T& value) const
  {
    return value >= m_start && value < m_stop;
  }

  KOKKOS_INLINE_FUNCTION auto kokkos_slice() const // TODO free function
  {
    return Kokkos::pair(m_start, m_stop);
  }

  friend std::ostream& operator<<(std::ostream& os, const Slice& slice)
  {
    os << slice.m_start << ':' << slice.m_stop;
    return os;
  }

private:

  T m_start;
  T m_stop;
};

/**
 * @brief 1D span specialization.
 */
template <typename T>
class Slice<T, SliceType::right_infinite> {
public:

  using size_type = T;
  static constexpr int n = 1;
  static constexpr SliceType type = SliceType::right_infinite;

  KOKKOS_INLINE_FUNCTION Slice(const T& start, std::nullptr_t) : m_start(start) {}

  KOKKOS_INLINE_FUNCTION auto operator()(auto... args) const&
  {
    return slice_emplace(*this, args...);
  }

  KOKKOS_INLINE_FUNCTION auto operator()(auto... args) &&
  {
    return slice_emplace(LINX_MOVE(*this), args...);
  }

  KOKKOS_INLINE_FUNCTION T start() const
  {
    return m_start;
  }

  KOKKOS_INLINE_FUNCTION bool contains(const T& value) const
  {
    return value >= m_start;
  }

  friend std::ostream& operator<<(std::ostream& os, const Slice& slice)
  {
    os << slice.m_start << ':';
    return os;
  }

private:

  T m_start;
};

/**
 * @brief Shortcut for right-open slice.
 */
template <typename T>
using Span = Slice<T, SliceType::right_open>;

/**
 * @brief Get the Kokkos execution policy of a span.
 */
template <typename TSpace, std::integral T>
auto kokkos_execution_policy(const Slice<T, SliceType::right_open>& region)
{
  return Kokkos::RangePolicy<TSpace>(region.start(), region.stop());
}

/**
 * @brief Apply a function to each element of the domain.
 * @tparam TSpace The execution space
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, std::integral T>
void for_each(const std::string& label, const Slice<T, SliceType::right_open>& region, auto&& func)
{
  Kokkos::parallel_for(label, kokkos_execution_policy<TSpace>(region), LINX_FORWARD(func));
}

/**
 * @brief Make a 1D slice clamped between bounds.
 */
template <typename T>
Slice<T, SliceType::right_open> clamp(const Slice<T, SliceType::unbounded>&, auto start, auto stop)
{
  return {static_cast<T>(start), static_cast<T>(stop)};
}

/**
 * @brief Make a 1D slice clamped between bounds.
 */
template <typename T>
const Slice<T, SliceType::singleton>& clamp(const Slice<T, SliceType::singleton>& slice, auto start, auto stop)
{
  OutOfBounds<'[', ')'>::may_throw("slice index", slice.value(), {start, stop});
  return slice;
}

/**
 * @brief Make a 1D slice clamped between bounds.
 */
template <typename T>
Slice<T, SliceType::right_open> clamp(const Slice<T, SliceType::right_open>& slice, auto start, auto stop)
{
  return {std::max<T>(slice.start(), start), std::min<T>(slice.stop(), stop)};
}

} // namespace Linx

#endif
