// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_SUBRASTER_H
#define _LITLRASTER_SUBRASTER_H

#include "LitlRaster/Box.h"
#include "LitlRaster/Raster.h"

#include <algorithm> // accumulate
#include <functional> // multiplies

namespace Litl {

/**
 * @ingroup data_classes
 * @brief A view of a raster region.
 * @tparam T The value type
 * @tparam TRaster The raster or extrapolator type
 * @tparam TRegion The region type
 * @details
 * As opposed to a raster, values of a subraster are generally not contiguous in memory:
 * they are piece-wise contiguous only when the region is a `Box` and sometimes not even piece-wise contiguous.
 * When a region is indeed contiguous, it is better to rely on a `PtrRaster` instead: see `Raster::section()`.
 * 
 * Whatever the region type, subrasters are iterable,
 * and the iterator depends on the region type in order to maximize performance.
 */
template <typename T, typename TRaster, typename TRegion = Box<TRaster::Dimension>>
class Subraster {

public:
  /**
   * @brief The value type.
   */
  using Value = T;

  /**
   * @brief The parent type (a raster or extrapolator).
   */
  using Parent = TRaster;

  /**
   * @brief The region type.
   */
  using Region = TRegion;

  /**
   * @brief The dimension.
   */
  static constexpr Index Dimension = Parent::Dimension;

  /**
   * @brief The iterator.
   */
  template <typename U>
  class Iterator;

  /// @{
  /// @group_construction

  /**
   * @brief Constructor.
   */
  Subraster(Parent& parent, TRegion region) : m_parent(parent), m_region(std::move(region)) {}

  /// @group_properties

  /**
   * @brief Get the subraster bounding box.
   */
  Position<Dimension> box() const {
    return box(m_region);
  }

  /**
   * @brief Get the number of pixels in the subraster.
   */
  std::size_t size() const {
    return m_region.size();
  }

  /**
   * @brief Get the region.
   */
  const TRegion& region() const {
    return m_region;
  }

  /**
   * @brief Access the parent raster or extrapolator.
   */
  const Parent& parent() const {
    return m_parent;
  }

  /**
   * @copydoc parent()
   */
  Parent& parent() {
    return m_parent;
  }

  /// @group_elements

  /**
   * @brief Constant iterator to the front pixel.
   */
  Iterator<const Value> begin() const {
    return Iterator<const Value>::begin(m_parent, m_region);
  }

  /**
   * @brief Iterator to the front pixel.
   */
  Iterator<Value> begin() {
    return Iterator<Value>::begin(m_parent, m_region);
  }

  /**
   * @brief Constant end iterator.
   */
  Iterator<const Value> end() const {
    return Iterator<const Value>::end(m_parent, m_region);
  }

  /**
   * @brief End iterator.
   */
  Iterator<Value> end() {
    return Iterator<Value>::end(m_parent, m_region);
  }

  /// @}

private:
  /**
   * @brief The parent raster.
   */
  Parent& m_parent;

  /**
   * @brief The region.
   */
  Box<Dimension> m_region;
};

} // namespace Litl

#include "LitlRaster/SubrasterIterator.h"

#endif
