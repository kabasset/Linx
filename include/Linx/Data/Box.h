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

  static constexpr int n = std::max(Start::n, Stop::n); ///< The dimension parameter
  // FIXME n = Shape::n and adapt either Shape definition or Vector arithmetics
  using size_type = typename Shape::element_type; ///< The coordinate type, which may be non-integral

  static constexpr bool static_rank_flag = (n >= 0); ///< Static rank flag
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
    for (int i = 0; i < rank(); ++i) {
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

} // namespace Linx

#include "Linx/Data/Box/arithmetics.h"
#include "Linx/Data/Box/creation.h"
#include "Linx/Data/Box/funcs.h"
#include "Linx/Data/Box/types.h"

#endif
