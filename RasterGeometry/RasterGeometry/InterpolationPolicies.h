// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERGEOMETRY_INTERPOLATION_POLICIES_H
#define _RASTERGEOMETRY_INTERPOLATION_POLICIES_H

#include "Raster/Raster.h"

namespace Cnes {

/**
 * @ingroup interpolation
 * @brief Do not check bounds.
 */
struct NeverOutOfBounds {

  /**
   * @brief Directly call `raster[position]`.
   */
  template <typename TRaster>
  inline typename TRaster::value_type& at(TRaster& raster, const Position<TRaster::Dim>& position) {
    return raster[position];
  }
};

/**
 * @ingroup interpolation
 * @brief Constant, aka Dirichlet boundary conditions.
 */
template <typename T>
struct OutOfBoundsConstant {

  /**
   * @brief Return `value` if out of bounds.
   */
  template <typename TRaster>
  inline T& at(TRaster& raster, const Position<TRaster::Dim>& position) {
    if (raster.contains(position)) {
      return raster[position];
    }
    return value;
  }

  /**
   * @brief The extrapolation value.
   */
  T value {};
};

/**
 * @ingroup interpolation
 * @brief Nearest-neighbor interpolation or extrapolation, aka zero-flux Neumann boundary conditions.
 */
struct NearestNeighbor {

  /**
   * @brief Return the value at the nearest in-bounds position.
   */
  template <typename TRaster>
  inline typename TRaster::value_type& at(TRaster& raster, const Position<TRaster::Dim>& position) {
    auto in = clamp(position, raster.shape());
    return raster[in];
  }

  /**
   * @brief Return the value at the nearest integer position.
   */
  template <typename TRaster>
  inline typename TRaster::value_type& at(TRaster& raster, const Vector<double, TRaster::Dim>& position) {
    Position<TRaster::Dim> integral(position.size());
    std::transform(position.begin(), position.end(), integral.begin(), [](auto e) {
      return e + .5;
    });
    return raster[integral];
  }
};

} // namespace Cnes

#endif
