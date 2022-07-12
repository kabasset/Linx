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
 * @brief A subraster as a view of a raster region.
 * @details
 * As opposed to a Raster, values of a Subraster are generally not contiguous in memory:
 * they are piece-wise contiguous only.
 * When a region is indeed contiguous, it is better to rely on a PtrRaster instead.
 */
template <typename TRaster, typename T>
class Subraster {

public:
  using Parent = TRaster;
  using Value = T;
  static constexpr Index Dimension = TRaster::Dimension;

  class Iterator;

  /// @{
  /// @group_construction

  /**
   * @brief Constructor.
   */
  Subraster(Parent& parent, Box<Dimension> region) : m_raster(parent), m_region(std::move(region)) {}

  /// @group_properties

  /**
   * @brief Get the subraster shape.
   */
  Position<Dimension> shape() const {
    return m_region.shape();
  }

  /**
   * @brief The number of pixels in the subraster.
   */
  std::size_t size() const {
    return m_region.size();
  }

  /**
   * @brief Get the region.
   */
  const Box<Dimension>& region() const {
    return m_region;
  }

  /**
   * @brief Access the parent raster.
   */
  const Parent& raster() const {
    return m_raster;
  }

  /**
   * @copydoc raster()
   */
  Parent& raster() {
    return m_raster;
  }

  /// @group_elements

  /**
   * @brief Access the pixel at given position.
   */
  const Value& operator[](const Position<Dimension>& pos) const {
    return m_raster[pos + m_region.front()];
  }

  /**
   * @brief Pixel at given position.
   */
  Value& operator[](const Position<Dimension>& pos) {
    return m_raster[pos + m_region.front()];
  }

  /// @}

private:
  /**
   * @brief The parent raster.
   */
  Parent& m_raster;

  /**
   * @brief The region.
   */
  Box<Dimension> m_region;
};

} // namespace Litl

#include "LitlRaster/SubrasterIterator.h"

#endif
