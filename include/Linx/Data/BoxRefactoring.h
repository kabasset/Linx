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
#include "Linx/Data/Vector.h"

#include <Kokkos_Core.hpp>
#include <concepts>
#include <string>

namespace Linx {

/**
 * @ingroup regions
 * @relatesalso Window
 * @brief An ND bounding box, defined by its start (inclusive) and stop (exclusive) bounds.
 * 
 * @tparam TStart The start vector coefficients specification
 * @tparam TStop The stop vector coefficients specification
 * 
 * If start and stop vectors have integral coefficients,
 * the box can be iterated with `for_each()` and `kokkos_reduce()`, and patches can be created from the box.
 * 
 * @see `Patch`
 */
template <typename TStart = std::integer_sequence<int>, typename TStop = std::integer_sequence<int>>
class Box {
public:

  using Start = Vector<TStart>; ///< The start vector type
  using Stop = Vector<TStop>; ///< The stop vector type
  using Shape = decltype(std::declval<Stop>() - std::declval<Start>()); ///< The shape vector type
  static constexpr int n = Shape::n; ///< The dimension parameter
  using size_type = typename Shape::element_type; ///< The coordinate type, which may be non-integral

  /**
   * @brief Constructor.
   */
  constexpr Box(const Stop& stop) : m_start {}, m_stop(stop) {}

  /**
   * @brief Constructor.
   */
  constexpr Box(const Start& start, const Stop& stop) : m_start(start), m_stop(stop) {}

  /**
   * @brief The box rank.
   */
  KOKKOS_INLINE_FUNCTION constexpr auto rank() const
  {
    return std::max(m_start.size(), m_stop.size());
  }

  /**
   * @brief The box shape.
   */
  constexpr auto shape() const
  {
    return m_stop - m_start;
  }

  /**
   * @brief The start bound, inclusive.
   */
  KOKKOS_INLINE_FUNCTION const auto& start() const
  {
    return m_start;
  }

  /**
   * @brief The stop bound, exclusive.
   */
  KOKKOS_INLINE_FUNCTION const auto& stop() const
  {
    return m_stop;
  }

  /**
   * @brief The start bound along given axis.
   */
  KOKKOS_INLINE_FUNCTION constexpr auto start(std::integral auto i) const
  {
    return m_start[i];
  }

  /**
   * @copybrief start()
   */
  KOKKOS_INLINE_FUNCTION constexpr auto& start(std::integral auto i)
  {
    return m_start[i];
  }

  /**
   * @brief The stop bound along given axis.
   */
  KOKKOS_INLINE_FUNCTION constexpr auto stop(std::integral auto i) const
  {
    return m_stop[i];
  }

  /**
   * @copybrief stop()
   */
  KOKKOS_INLINE_FUNCTION constexpr auto& stop(std::integral auto i)
  {
    return m_stop[i];
  }

  /**
   * @brief The extent along given axis.
   */
  KOKKOS_INLINE_FUNCTION constexpr auto extent(std::integral auto i) const
  {
    return m_stop[i] - m_start[i];
  }

  /**
   * @brief The product of the extents.
   */
  KOKKOS_INLINE_FUNCTION constexpr auto size() const
  {
    size_type out = 1;
    for (std::size_t i = 0; i < m_start.size(); ++i) {
      out *= extent(i);
    }
    return out;
  }

  /**
   * @brief Check whether two boxes are equal.
   */
  constexpr bool operator==(const auto& other) const
  {
    return m_start == other.start() && m_stop == other.stop();
  }

  /**
   * @brief Check whether two boxes are different.
   */
  constexpr bool operator!=(const auto& other) const
  {
    return not(*this == other);
  }

  /**
   * @brief Check whether a position lies inside the box.
   */
  bool contains(const LegacyArray auto& position) const
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
    return contains(std::array {is...});
  }

private:

  Start m_start; ///< The start bound
  Stop m_stop; ///< The stop bound
};

/**
 * @brief Start at origin.
 */
template <typename T, int N>
Box(T (&&)[N]) -> Box<std::integer_sequence<int>, T[N]>;

/**
 * @brief Specify start and stop bounds.
 */
template <typename T0, int N0, typename T1, int N1>
Box(T0 (&&)[N0], T1 (&&)[N1]) -> Box<T0[N0], T1[N1]>;

/**
 * @brief Create a box which starts at origin.
 */
template <auto... Args>
constexpr auto shape(auto... args)
{
  return Box(vec<Args...>(args...));
}

/**
 * @brief Create a box centered at origin.
 */
constexpr auto cube(auto radius)
{
  return Box(-radius, radius + 1);
}

/**
 * @brief Create the dilation of a box by a given margin.
 */
template <typename TStart, typename TStop>
constexpr auto
dilate(const Box<TStart, TStop>& box, const std::convertible_to<typename Box<TStart, TStop>::size_type> auto& margin)
{
  return Box(box.start() - margin, box.stop() + margin);
}

/**
 * @brief Create the erosion of a box by a given margin.
 */
template <typename TStart, typename TStop>
constexpr auto
erode(const Box<TStart, TStop>& box, const std::convertible_to<typename Box<TStart, TStop>::size_type> auto& margin)
{
  return Box(box.start() + margin, box.stop() - margin);
}

/**
 * @brief Create the dilation of a box by a given margin.
 */
template <typename TStart, typename TStop, typename TRhs> // TODO BoundedRegion TRhs
  requires requires(const TRhs& rhs)
  {
    bbox(rhs); // TODO -> Box
  }
constexpr auto dilate(const Box<TStart, TStop>& box, const TRhs& margin)
{
  auto margin_box = bbox(margin);
  return Box(box.start() + margin_box.start(), box.stop() + (margin_box.stop() - 1));
}

/**
 * @brief Create the erosion of a box by a given margin.
 */
template <typename TStart, typename TStop, typename TRhs> // TODO BoundedRegion TRhs
  requires requires(const TRhs& rhs)
  {
    bbox(rhs); // TODO -> Box
  }
constexpr auto erode(const Box<TStart, TStop>& box, const TRhs& margin)
{
  auto margin_box = bbox(margin);
  return Box(box.start() - margin_box.start(), box.stop() - (margin_box.stop() - 1));
}

/**
 * @relatesalso Box
 * @brief Get the 1D span along the i-th axis.
 */
template <int I, typename TStart, typename TStop>
constexpr auto get(const Box<TStart, TStop>& box)
{
  return Slice(box.start(I), box.stop(I));
}

namespace Impl {

template <typename TSlice, std::size_t... Is>
constexpr auto box_impl(const TSlice& slice, std::index_sequence<Is...>)
{
  return Box(vec(get<Is>(slice).start()...), vec(get<Is>(slice).stop()...));
}

} // namespace Impl

/**
 * @relatesalso Box
 * @brief Get the bounding box of a box.
 * 
 * This function is a no-op, it merely forwards its input.
 */
template <typename TStart, typename TStop>
constexpr const Box<TStart, TStop>& bbox(const Box<TStart, TStop>& in)
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
constexpr auto bbox(const Slice<T, TFuncs...>& slice)
{
  return Impl::box_impl(slice, std::make_index_sequence<sizeof...(TFuncs)>());
}

/**
 * @relatesalso Slice
 * @relatesalso Box
 * @brief Make a slice clamped by a region.
 * 
 * The region may be of higher rank than the slice: extra dimensions are ignored.
 */
template <typename T, typename... TFuncs>
constexpr auto
operator&(const Slice<T, TFuncs...>& slice, const auto& region) // FIXME requires region.start(i), regions.stop(i)
{
  constexpr auto last = sizeof...(TFuncs) - 1;
  if constexpr (last == 0) {
    return clamp(slice, region.start(0), region.stop(0));
  } else {
    return Slice(Forward(), slice.lower() & region, clamp(slice.last(), region.start(last), region.stop(last)));
  }
}

namespace Impl {

template <typename TSpace, typename TStart, typename TStop, std::size_t... Is>
auto kokkos_execution_policy_impl(const Box<TStart, TStop>& domain, std::index_sequence<Is...>)
{
  using Policy = Kokkos::MDRangePolicy<TSpace, Kokkos::Rank<Box<TStart, TStop>::n>, Kokkos::IndexType<Index>>;
  using Array = Policy::point_type;
  return Policy(Array {domain.start(Is)...}, Array {domain.stop(Is)...});
}

} // namespace Impl

/**
 * @brief Get the execution policy of a box.
 */
template <typename TSpace, typename TStart, typename TStop>
auto kokkos_execution_policy(const Box<TStart, TStop>& domain)
{
  // TODO support Properties?
  if constexpr (Box<TStart, TStop>::n == 1) {
    return Kokkos::RangePolicy<TSpace, Kokkos::IndexType<Index>>(domain.start(0), domain.stop(0));
  } else {
    return Impl::kokkos_execution_policy_impl<TSpace>(domain, std::make_index_sequence<Box<TStart, TStop>::n>());
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
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename TStart, typename TStop, typename TFunc>
void for_each(const std::string& label, const Box<TStart, TStop>& region, TFunc&& func)
{
#define LINX_CASE_RANK(n) \
  case n: \
    if constexpr (is_nary<TFunc, int, n>()) { \
      return Kokkos::parallel_for(label, kokkos_execution_policy<TSpace>(pad<n>(region)), LINX_FORWARD(func)); \
    } else { \
      return; \
    }

  if constexpr (Box<TStart, TStop>::n == -1) {
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
