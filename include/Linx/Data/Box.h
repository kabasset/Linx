// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_BOX_H
#define LINX_DATA_BOX_H

#include "Linx/Base/Containers.h"
#include "Linx/Base/Exceptions.h"
#include "Linx/Base/Functional.h"
#include "Linx/Base/Packs.h"
#include "Linx/Base/Slice.h"
#include "Linx/Base/Types.h"
#include "Linx/Base/concepts/Array.h"
#include "Linx/Data/Sequence.h"

#include <Kokkos_Core.hpp>
#include <concepts>
#include <string>

namespace Linx {

template <typename T, int N>
struct Shape : StrongType<GPosition<T, N>, struct ShapeTag> { // FIXME const GPosition&?
  using StrongType<GPosition<T, N>, ShapeTag>::StrongType;

  /**
   * @brief Compute the shape size.
   */
  T size() const
  {
    return product(this->value);
  }
};

template <typename T, int N>
Shape(T (&&)[N]) -> Shape<T, N>;

template <typename T, int N>
Shape(const GPosition<T, N>&) -> Shape<T, N>;

/**
 * @ingroup regions
 * @relatesalso Window
 * @brief An ND bounding box, defined by its start (inclusive) and stop (exclusive) bounds.
 * 
 * @tparam T The coordinate type
 * @tparam N The dimension parameter
 * 
 * If `T` is integral, the box can be iterated with `for_each()` and `kokkos_reduce()`,
 * and patches can be created from the box.
 * 
 * @see `Patch`
 */
template <typename T, int N>
class GBox {
public:

  static constexpr int n = N; ///< The dimension parameter
  using size_type = T; ///< The coordinate type, which may be non-integral
  using value_type = GPosition<T, N>; ///< The position type

  /**
   * @brief Constructor.
   */
  GBox() : GBox(std::abs(n)) {}

  /**
   * @copydoc GBox()
   */
  explicit GBox(std::integral auto size) : m_start("start", size), m_stop("stop", size) {}

  /**
   * @copydoc GBox()
   */
  GBox(const ArrayLike auto& start, const ArrayLike auto& stop) : GBox(std::size(start))
  {
    SizeMismatch::may_throw("bounds", rank(), start, stop);
    for (std::size_t i = 0; i < rank(); ++i) {
      m_start[i] = start[i];
      m_stop[i] = stop[i];
    }
  }

  /**
   * @copydoc GBox()
   */
  template <typename U>
  GBox(std::initializer_list<U> start, std::initializer_list<U> stop) : GBox(std::size(start))
  {
    SizeMismatch::may_throw("bounds", rank(), start, stop);
    auto start_it = start.begin();
    auto stop_it = stop.begin();
    for (std::size_t i = 0; i < rank(); ++i, ++start_it, ++stop_it) {
      m_start[i] = *start_it;
      m_stop[i] = *stop_it;
    }
  }

  /**
   * @copydoc GBox()
   */
  GBox(GPosition<size_type, n> start, Shape<size_type, n> shape) :
      m_start(LINX_MOVE(start)),
      m_stop(shape.value + m_start)
  {}

  /**
   * @brief The box rank.
   */
  KOKKOS_INLINE_FUNCTION auto rank() const
  {
    return m_start.size();
  }

  /**
   * @brief The box shape.
   */
  auto shape() const
  {
    return m_stop - m_start;
  }

  /**
   * @brief The start bound, inclusive.
   */
  const auto& start() const
  {
    return m_start;
  }

  /**
   * @brief The stop bound, exclusive.
   */
  const auto& stop() const
  {
    return m_stop;
  }

  /**
   * @brief The start bound along given axis.
   */
  KOKKOS_INLINE_FUNCTION auto start(std::integral auto i) const
  {
    return m_start[i];
  }

  /**
   * @copybrief start()
   */
  KOKKOS_INLINE_FUNCTION auto& start(std::integral auto i)
  {
    return m_start[i];
  }

  /**
   * @brief The stop bound along given axis.
   */
  KOKKOS_INLINE_FUNCTION auto stop(std::integral auto i) const
  {
    return m_stop[i];
  }

  /**
   * @copybrief stop()
   */
  KOKKOS_INLINE_FUNCTION auto& stop(std::integral auto i)
  {
    return m_stop[i];
  }

  /**
   * @brief The extent along given axis.
   */
  KOKKOS_INLINE_FUNCTION auto extent(std::integral auto i) const
  {
    return m_stop[i] - m_start[i];
  }

  /**
   * @brief The product of the extents.
   */
  KOKKOS_INLINE_FUNCTION auto size() const
  {
    T out = 1;
    for (std::size_t i = 0; i < m_start.size(); ++i) {
      out *= extent(i);
    }
    return out;
  }

  /**
   * @brief Check whether two boxes are equal.
   */
  bool operator==(const auto& other) const
  {
    return m_start == other.start() && m_stop == other.stop();
  }

  /**
   * @brief Check whether two boxes are different.
   */
  bool operator!=(const auto& other) const
  {
    return not(*this == other);
  }

  /**
   * @brief Check whether a position lies inside the box.
   */
  bool contains(const ArrayLike auto& position) const
  {
    SizeMismatch::may_throw("position", rank(), position);
    for (std::size_t i = 0; i < rank(); ++i) {
      if (position[i] < m_start[i] || position[i] > m_stop[i]) {
        return false;
      }
    }
    return true;
  }

  /**
   * @copydoc contains()
   */
  bool contains(std::integral auto... is) const
  {
    return contains(value_type {is...});
  }

  /**
   * @brief Shrink the box inside another box (i.e. get the intersection of both).
   */
  template <typename U, int M>
  GBox& operator&=(const GBox<U, M>& rhs)
  {
    // FIXME assert rank() == rhs.rank()?
    for (std::size_t i = 0; i < rank(); ++i) {
      m_start[i] = std::max<size_type>(m_start[i], rhs.start(i));
      m_stop[i] = std::min<size_type>(m_stop[i], rhs.stop(i));
    }
    return *this;
  }

  /**
   * @brief Minimally grow the box to include another box (i.e. get the minimum box which contains both).
   */
  template <typename U, int M>
  [[deprecated]] GBox& operator|=(const GBox<U, M>& rhs)
  {
    // FIXME assert rank() == rhs.rank()?
    for (std::size_t i = 0; i < rank(); ++i) {
      m_start[i] = std::min<size_type>(m_start[i], rhs.start(i));
      m_stop[i] = std::max<size_type>(m_stop[i], rhs.stop(i));
    }
    return *this;
  }

  /**
   * @brief Grow the box by a given margin.
   */
  template <typename U, int M>
  GBox& operator+=(const GBox<U, M>& margin)
  {
    // FIXME allow N=-1
    m_start += pad<n>(margin.start());
    m_stop += pad<n>(margin.stop());
    return *this;
  }

  /**
   * @brief Shrink the box by a given margin.
   */
  template <typename U, int M>
  GBox& operator-=(const GBox<U, M>& margin)
  {
    // FIXME allow N=-1
    m_start -= pad<n>(margin.start());
    m_stop -= pad<n>(margin.stop());
    return *this;
  }

  /**
   * @brief Translate the box by a given vector.
   */
  GBox& operator+=(const ArrayLike auto& vector)
  {
    // FIXME allow N=-1
    m_start += pad<n>(vector);
    m_stop += pad<n>(vector);
    return *this;
  }

  /**
   * @brief Translate the box by the opposite of a given vector.
   */
  GBox& operator-=(const ArrayLike auto& vector)
  {
    // FIXME allow N=-1
    m_start -= pad<n>(vector);
    m_stop -= pad<n>(vector);
    return *this;
  }

  /**
    * @brief Add a scalar to each coordinate.
    */
  GBox& operator+=(size_type scalar)
  {
    m_start += scalar;
    m_stop += scalar;
    return *this;
  }

  /**
   * @brief Subtract a scalar to each coordinate.
   */
  GBox& operator-=(size_type scalar)
  {
    m_start -= scalar;
    m_stop -= scalar;
    return *this;
  }

  /**
   * @brief Add 1 to each coordinate.
   */
  GBox& operator++()
  {
    return *this += 1;
  }

  GBox operator++(int)
  {
    GBox out = +(*this);
    ++(*this);
    return out;
  }

  /**
   * @brief Subtract 1 to each coordinate.
   */
  GBox& operator--()
  {
    return *this -= 1;
  }

  GBox operator--(int)
  {
    GBox out = +(*this);
    --(*this);
    return out;
  }

  /**
   * @brief Copy.
   */
  GBox operator+() const
  {
    return {+m_start, +m_stop};
  }

  /**
   * @brief Invert the sign of each coordinate.
   */
  GBox operator-() const
  {
    // FIXME swap bounds?
    return {-m_start, -m_stop};
  }

  /**
   * @brief Multiply each coordinate.
   */
  GBox operator*=(size_type scalar)
  {
    // FIXME handle negative scalar?
    m_start *= scalar;
    m_stop *= scalar;
    return *this;
  }

  /**
   * @brief Divide each coordinate.
   */
  GBox operator/=(size_type scalar)
  {
    // FIXME handle negative scalar?
    m_start /= scalar;
    m_stop /= scalar;
    return *this;
  }

  /**
   * @brief Equality.
   */
  template <typename U, int M>
  bool operator==(const GBox<U, M>& rhs) const
  {
    return m_start == rhs.m_start && m_stop == rhs.m_stop;
  }

  /**
   * @brief Inequality.
   */
  template <typename U, int M>
  bool operator!=(const GBox<U, M>& rhs) const
  {
    return not(*this == rhs);
  }

private:

  value_type m_start; ///< The start bound
  value_type m_stop; ///< The stop bound
};

GBox() -> GBox<int, 0>;

template <typename T, int N>
GBox(T (&&)[N]) -> GBox<T, N>;

template <typename T, int N>
GBox(T (&&)[N], T (&&)[N]) -> GBox<T, N>;

template <typename T, int N>
GBox(const GPosition<T, N>&) -> GBox<T, N>;

template <typename T, int N>
GBox(const GPosition<T, N>&, const GPosition<T, N>&) -> GBox<T, N>;

template <typename T, int N>
GBox(const GPosition<T, N>&, const Shape<T, N>&) -> GBox<T, N>;

template <typename T, int N>
GBox(T (&&)[N], const Shape<T, N>&) -> GBox<T, N>;

template <int M, typename T, int N>
GBox<T, M> pad(const GBox<T, N>& in)
{
  return GBox<T, M>({pad<M>(in.start()), pad<M>(in.stop())});
}

/**
 * @relatesalso GBox
 */
template <typename T, int N>
GBox<T, N> operator+(const GBox<T, N>& lhs, const auto& rhs)
{
  auto out = +lhs;
  out += rhs;
  return out;
}

/**
 * @relatesalso GBox
 */
template <typename T, int N>
GBox<T, N> operator-(const GBox<T, N>& lhs, const auto& rhs)
{
  auto out = +lhs;
  out -= rhs;
  return out;
}

/**
 * @relatesalso GBox
 */
template <typename T, int N>
GBox<T, N> operator*(const GBox<T, N>& lhs, const auto& rhs)
{
  auto out = +lhs;
  out *= rhs;
  return out;
}

/**
 * @relatesalso GBox
 */
template <typename T, int N>
GBox<T, N> operator/(const GBox<T, N>& lhs, const auto& rhs)
{
  auto out = +lhs;
  out /= rhs;
  return out;
}

/**
 * @relatesalso GBox
 */
template <typename T, int N, typename U, int M>
GBox<T, N> operator&(const GBox<T, N>& lhs, const GBox<U, M>& rhs)
{
  auto out = +lhs;
  out &= rhs;
  return out;
}

/**
 * @relatesalso GBox
 * @brief Get the 1D span along the i-th axis.
 */
template <int I, typename T, int N>
Span<T> get(const GBox<T, N>& box)
{
  return Slice(box.start(I), box.stop(I));
}

namespace Impl {

template <typename TSlice, std::size_t... Is>
auto box_impl(const TSlice& slice, std::index_sequence<Is...>)
{
  using T = typename TSlice::size_type;
  static constexpr int N = sizeof...(Is);
  return GBox<T, N>({get<Is>(slice).start()...}, {get<Is>(slice).stop()...});
}

} // namespace Impl

/**
 * @relatesalso GBox
 * @brief Get the bounding box of a box.
 * 
 * This function is a no-op, it merely forwards its input.
 */
template <typename T, int N>
const GBox<T, N>& bbox(const GBox<T, N>& in)
{
  return in;
}

/**
 * @relatesalso Slice
 * @brief Get the bounding box of a slice.
 * 
 * @warning Unbounded slices are not supported, and singleton slices must be integral.
 */
template <typename T, typename... TFuncs>
GBox<T, sizeof...(TFuncs)> bbox(const Slice<T, TFuncs...>& slice)
{
  static constexpr int n = sizeof...(TFuncs);
  return Impl::box_impl(slice, std::make_index_sequence<n>());
}

/**
 * @relatesalso Slice
 * @relatesalso GBox
 * @brief Make a slice clamped by a region.
 * 
 * The region may be of higher rank than the slice: extra dimensions are ignored.
 */
template <typename T, typename... TFuncs>
auto operator&(const Slice<T, TFuncs...>& slice, const auto& region) // FIXME requires region.start(i), regions.stop(i)
{
  static constexpr auto last = sizeof...(TFuncs) - 1;
  if constexpr (last == 0) {
    return clamp(slice, region.start(0), region.stop(0));
  } else {
    return Slice(Forward(), slice.lower() & region, clamp(slice.last(), region.start(last), region.stop(last)));
  }
}

namespace Impl {

template <typename TSpace, typename T, int N, std::size_t... Is>
auto kokkos_execution_policy_impl(const GBox<T, N>& domain, std::index_sequence<Is...>)
{
  using Policy = Kokkos::MDRangePolicy<TSpace, Kokkos::Rank<N>, Kokkos::IndexType<Index>>;
  using Array = Policy::point_type;
  return Policy(Array {domain.start(Is)...}, Array {domain.stop(Is)...});
}

} // namespace Impl

/**
 * @ingroup regions
 * @brief Shortcut for indexing.
 */
template <int N>
using Box = GBox<Index, N>;

/**
 * @brief Get the execution policy of a box.
 */
template <typename TSpace, typename T, int N>
auto kokkos_execution_policy(const GBox<T, N>& domain)
{
  // TODO support Properties?
  if constexpr (N == 1) {
    return Kokkos::RangePolicy<TSpace, Kokkos::IndexType<Index>>(domain.start(0), domain.stop(0));
  } else {
    return Impl::kokkos_execution_policy_impl<TSpace>(domain, std::make_index_sequence<N>());
  }
}

/**
 * @ingroup regions
 * @brief Apply a function to each position of a region.
 * 
 * @param label Some label for debugging
 * @param region The region
 * @param func The function
 * 
 * The coordinate type must be integral and the function must take integral coordinates as input.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T, int N, typename TFunc>
void for_each(const std::string& label, const GBox<T, N>& region, TFunc&& func)
{
#define LINX_CASE_RANK(n) \
  case n: \
    if constexpr (is_nadic<TFunc(), int, n>()) { \
      return Kokkos::parallel_for(label, kokkos_execution_policy<TSpace>(pad<n>(region)), LINX_FORWARD(func)); \
    } else { \
      return; \
    }

  if constexpr (N == -1) {
    switch (region.rank()) {
      case 0:
        return;
        LINX_CASE_RANK(1)
        LINX_CASE_RANK(2)
        LINX_CASE_RANK(3)
        LINX_CASE_RANK(4)
        LINX_CASE_RANK(5)
        LINX_CASE_RANK(6)
      default:
        throw Linx::OutOfBounds("Dynamic rank", region.rank(), Segment<int>(0, 6));
    }
  } else {
    Kokkos::parallel_for(label, kokkos_execution_policy<TSpace>(region), LINX_FORWARD(func));
  }

#undef LINX_CASE_RANK
}

} // namespace Linx

#endif
