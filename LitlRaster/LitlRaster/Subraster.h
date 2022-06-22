// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_SUBRASTER_H
#define _LITLRASTER_SUBRASTER_H

#include "LitlRaster/Raster.h"
#include "LitlRaster/Region.h"

#include <algorithm> // accumulate
#include <functional> // multiplies

namespace Litl {

/// @cond

/**
 * @ingroup data_classes
 * @brief A subraster as a view of a raster region.
 * @details
 * As opposed to a Raster, values of a Subraster are generally not contiguous in memory:
 * they are piece-wise contiguous only.
 * When a region is indeed contiguous, it is better to rely on a PtrRaster instead.
 */
template <typename T, Index N, typename THolder> // FIXME simplify as TParent
class Subraster {

public:
  using Parent = Raster<T, N, THolder>;

  /**
   * @brief Constructor.
   */
  Subraster(const Parent& parent, const Region<N>& region) : m_cParent(&parent), m_parent(nullptr), m_region(region) {}

  /**
   * @brief Constructor.
   */
  Subraster(Parent& parent, const Region<N>& region) : m_cParent(&parent), m_parent(&parent), m_region(region) {}

  /**
   * @brief The subraster shape.
   */
  Position<N> shape() const {
    return m_region.shape();
  }

  /**
   * @brief The number of pixels in the subraster.
   */
  Index size() const {
    return m_region.size();
  }

  /**
   * @brief The parent raster.
   */
  const Parent& parent() const {
    return *m_cParent;
  }

  /**
   * @copydoc parent()
   */
  Parent& parent() {
    return *m_parent;
  }

  /**
   * @brief The region.
   */
  const Region<N>& region() const {
    return m_region;
  }

  /**
   * @brief Pixel at given position.
   */
  const T& operator[](const Position<N>& pos) const {
    return (*m_cParent)[pos + m_region.front];
  }

  /**
   * @brief Pixel at given position.
   */
  T& operator[](const Position<N>& pos) {
    return (*m_parent)[pos + m_region.front];
  }

private:
  /**
   * @brief Read-only pointer to the raster.
   */
  const Parent* m_cParent;

  /**
   * @brief Read/write pointer to the raster.
   */
  Parent* m_parent;

  /**
   * @brief The region.
   */
  Region<N> m_region;
};

/// @endcond

} // namespace Litl

#endif
