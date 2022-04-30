// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERGEOMETRY_EXTRAPOLATION_H
#define _RASTERGEOMETRY_EXTRAPOLATION_H

namespace Cnes {

/**
 * @brief Throw an error if trying to extrapolate.
 */
struct NoExtrapolation {};

/**
 * @brief Extrapolate by cropping kernels.
 */
struct CropExtrapolation {};

/**
 * @brief Constant, aka Dirichlet boundary conditions.
 */
template <typename T>
struct ConstantExtrapolation {
  /**
   * @brief The extrapolation value.
   */
  T value {};
};

/**
 * @brief Nearest-neighbor extrapolatoin, aka zero-flux Neumann boundary conditions.
 */
struct NearestNeighborExtrapolation {};

} // namespace Cnes

#endif
