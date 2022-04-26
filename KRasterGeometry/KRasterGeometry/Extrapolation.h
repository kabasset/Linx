// Copyright (C) 2022, Antoine Basset
// This file is part of Kast.Raster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _KRASTERGEOMETRY_EXTRAPOLATION_H
#define _KRASTERGEOMETRY_EXTRAPOLATION_H

namespace Kast {

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

} // namespace Kast

#endif
