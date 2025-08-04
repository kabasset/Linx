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

namespace Impl {

template <int NStart, int NStop>
static constexpr int static_rank()
{
  if constexpr (NStart == 0) {
    return NStop;
  }
  if constexpr (NStart == 0) {
    return NStop;
  }
  return std::max(NStart, NStop);
}

} // namespace Impl

/**
 * @ingroup regions
 * @brief An ND axis-aligned bounding box, defined by its start (inclusive) and stop (exclusive) bounds.
 * @tparam TStart The start vector coefficients specification
 * @tparam TStop The stop vector coefficients specification
 * 
 * If the start and stop vectors are not statically empty, then they must have the same size.
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

  static constexpr int n = Impl::static_rank<Start::n, Stop::n>(); ///< The dimension parameter

  using size_type = typename Shape::element_type; ///< The coordinate type, which may be non-integral
  using value_type = const Vector<std::conditional_t<(n >= 0), size_type[n], size_type*>>; ///< The value type
  using element_type = std::remove_cvref_t<value_type>; ///< The element type

  static constexpr bool static_rank_flag = Shape::static_rank_flag; ///< Static rank flag
  static constexpr bool static_flag = Shape::static_flag; ///< Static bounds flag

  /**
   * @brief Default constructor.
   * 
   */
  constexpr Box() : m_start {}, m_stop {} {}

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
  KOKKOS_INLINE_FUNCTION constexpr const auto& start() const
  {
    return m_start;
  }

  /**
   * @brief The stop bound, exclusive.
   */
  KOKKOS_INLINE_FUNCTION constexpr const auto& stop() const
  {
    return m_stop;
  }

  /**
   * @brief The finish bound, inclusive.
   */
  KOKKOS_INLINE_FUNCTION constexpr auto finish() const
  {
    return m_stop - 1;
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
   * @brief Product of the extents, may be negative.
   */
  KOKKOS_INLINE_FUNCTION constexpr auto ssize() const
  {
    size_type out = 1;
    for (int i = 0; i < rank(); ++i) {
      out *= extent(i);
    }
    return out;
  }

  /**
   * @brief Unsigned size.
   */
  KOKKOS_INLINE_FUNCTION constexpr auto size() const
  {
    auto s = ssize();
    return s <= 0 ? std::size_t(0) : static_cast<std::size_t>(s);
  }

  /**
   * @brief Check whether two boxes are equal.
   */
  constexpr bool operator==(const Specialization<Linx::Box> auto& other) const
  {
    return m_start == other.start() && m_stop == other.stop();
  }

  /**
   * @brief Check whether two boxes are different.
   */
  constexpr bool operator!=(const Specialization<Linx::Box> auto& other) const
  {
    return not(*this == other);
  }

  /**
   * @brief Check whether a position lies inside the box.
   */
  bool contains(const auto& position) const
  { // FIXME Subscriptable<size_t>? Indexed?
    SizeMismatch::may_throw("position", rank(), position);
    for (int i = 0; i < rank(); ++i) {
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

} // namespace Linx

#include "Linx/Data/Box/BoxIterator.h"
#include "Linx/Data/Box/arithmetics.h"
#include "Linx/Data/Box/creation.h"
#include "Linx/Data/Box/funcs.h"
#include "Linx/Data/Box/types.h"

#endif
