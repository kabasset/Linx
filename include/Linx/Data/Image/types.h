// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_IMAGE_TYPES_H
#define LINX_DATA_IMAGE_TYPES_H

#include "Linx/Data/Box.h"

#include <Kokkos_Core.hpp>

namespace Linx {

/**
 * @brief Default raster domain instance.
 */
template <int N>
auto default_raster_domain()
{
  if constexpr (N == -1) {
    return Shape<Index*>();
  } else {
    return Shape<Index[N]>();
  }
}

/**
 * @brief Default raster domain type.
 */
template <int N>
using RasterDomain = decltype(default_raster_domain<N>());

/**
 * @brief Default raster view instance.
 */
template <typename T, int N>
auto default_raster_view()
{
  return default_image_view<T, RasterDomain<N>, Kokkos::LayoutLeft, Kokkos::HostSpace>();
}

/**
 * @brief Default raster view type.
 */
template <typename T, int N>
using RasterView = decltype(default_raster_view<T, N>());

/**
 * @brief Contiguous image on host with row-major ordering.
 * 
 * This specialization is mostly provided for interfacing with legacy code.
 * Row-major ordering means that the elements are contiguous along the first index,
 * which is conventionally considered to be the index along a row:
 * 
 * \code
 * Raster<int, 2> raster(shape);
 * assert(&raster(x, y) + 1 == &raster(x + 1, y));
 * \endcode
 * 
 * Said otherwise, the stride along axis 0 is 1.
 */
template <typename T, int N = 2>
using Raster = Image<T, RasterDomain<N>, RasterView<T, N>>;

} // namespace Linx

#endif
