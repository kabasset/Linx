// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_TILING_H
#define LINX_DATA_TILING_H

#include "Linx/Data/Image.h"
#include "Linx/Data/Line.h"
#include "Linx/Data/Patch.h"
#include "Linx/Data/Sequence.h"

#include <Kokkos_Core.hpp>

namespace Linx {

template <int I, typename TIn>
using ProfileAlong = Patch<TIn, Line<int, I, TIn::n>>;

/**
 * @brief Get the collection of all the profiles of an image along a given axis.
 * @tparam I The profile axis.
 * 
 * A profile is a line-based patch.
 * 
 * \code
 * for (const auto& column : profiles<1>(image)) {
 *   // Do something with the column
 * }
 * \endcode
 * 
 * @see `rows()`
 */
template <int I, typename TIn>
auto profiles(const TIn& in) // FIXME profiles_along?
{
  auto fronts = in.domain();
  fronts.stop(I) = fronts.start(I) + 1; // Flat along I
  const auto& start = fronts.start();
  auto stop = start + 1;
  stop[I] = in.stop(I); // Non-flat only along I
  auto profile = Box(start, stop);
  auto out = std::vector(fronts.size(), Patch(in, profile));
  auto raster = wrap(out.data(), fronts);
  Linx::for_each<Kokkos::Serial>(
      "profiles",
      fronts, // FIXME handle potential offset
      [&](auto... is) { /* FIXME raster(is...).shift(is...); */ }); // This is serial for now, no KOKKOS_LAMBDA needed
  return out; // FIXME return raster somehow?
}

/**
 * @brief Get the collection of all the rows of an image.
 * 
 * \code
 * for (const auto& row : rows(image)) {
 *   // Do something with the row
 * }
 * \endcode
 * 
 * @see `profiles()`
 */
template <typename TIn>
auto rows(const TIn& in)
{
  return profiles<0>(in);
}

} // namespace Linx

#endif
