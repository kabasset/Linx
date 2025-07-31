// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_IMAGE_TYPES_H
#define LINX_DATA_IMAGE_TYPES_H

namespace Linx {

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
template <typename T, typename TDomain>
using Raster = Image<T, TDomain, ImageContainer<T, TDomain, Kokkos::LayoutLeft, Kokkos::HostSpace>>;

} // namespace Linx

#endif
