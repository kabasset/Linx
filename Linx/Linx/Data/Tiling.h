/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_TILING_H
#define _LINXDATA_TILING_H

#include "Linx/Data/Grid.h"
#include "Linx/Data/Raster.h"

namespace Linx {

/**
 * @ingroup regions
 * @brief Iterator over the tiles of a raster.
 * @see tiles()
 */
template <typename TRaster, Index M>
class TileGenerator;

/**
 * @ingroup regions
 * @brief Get a box-patch partitioning of a raster.
 * 
 * The input raster domain is partitioned into boxes of given shape,
 * except at its upper limits where the boxes may be clamped inside the input domain.
 * A range over the corresponding patches is returned.
 */
template <typename TRaster, Index M>
auto tiles(TRaster& in, Position<M> shape) {
  return TileGenerator<TRaster, M>(in, std::move(shape));
}

/**
 * @ingroup regions
 * @brief Iterator over the sections of a raster.
 * @see sections()
 */
template <typename TRaster>
class SectionGenerator;

/**
 * @ingroup regions
 * @brief Get a slicing of a raster as a range of cross-sections of given thickness.
 * 
 * The input raster domain is sliced along its last axis into sections of given thickness,
 * except for the last section which may be thinner.
 * A range over the sections is returned.
 */
template <typename TRaster>
auto sections(TRaster& in, Index thickness = 1) {
  using TSection = std::decay_t<decltype(in.section(0, thickness - 1))>;
  std::vector<TSection> out;
  const auto back = in.length(in.dimension() - 1); // FIXME implement length(-1)
  for (Index i = 0; i <= back; i += thickness) {
    out.push_back(in.section(i, std::min(i + thickness, back) - 1));
  }
  return out;
}

/**
 * @ingroup regions
 * @brief Iterator over the `I`-th axis-aligned profiles of a raster.
 */
template <Index I, typename TRaster>
class ProfileGenerator;

/**
 * @ingroup regions
 * @brief Get a line-patch partitioning of a raster.
 * @tparam I The index of the axis
 * 
 * The input raster domain is partitioned into maximal lines oriented along the `I`-th axis.
 * A range over the corresponding patches is returned.
 */
template <Index I, typename TRaster>
auto profiles(TRaster& in) {
  using TPatch = std::decay_t<decltype(in.patch(Line<I, TRaster::Dimension>()))>;
  const auto domain = in.domain();
  const auto size = domain.template length<I>();
  const auto step = domain.step();
  const auto plane = project(domain, I);
  Raster<TPatch, TRaster::Dimension> out(plane.shape());
  auto frontIt = plane.begin();
  for (auto& e : out) {
    e = in.patch(Line<I, TRaster::Dimension>::fromSize(*frontIt, size, step[I]));
    ++frontIt;
  }
  return out;
}

/**
 * @ingroup regions
 * @brief Iterator over the rows of a raster.
 */
template <typename TRaster>
class RowGenerator;

/**
 * @ingroup regions
 * @brief Get a line-section partitioning of a raster.
 * 
 * This function is similar to `profiles<0>()` but parts are `PtrRaster`s instead of `Patch`es
 * since data is necessarily contiguous along axis 0.
 * 
 * @see profiles()
 */
template <typename TRaster>
auto rows(TRaster& in) {
  using T = std::conditional_t<std::is_const<TRaster>::value, const typename TRaster::Value, typename TRaster::Value>;
  using TRow = PtrRaster<T, 1>;
  const auto domain = in.domain();
  const auto size = domain.template length<0>();
  const auto plane = project(domain);
  auto patch = in.patch(plane);
  Raster<TRow, TRaster::Dimension> out(plane.shape());
  auto it = patch.begin();
  for (auto& e : out) {
    e = PtrRaster<T, 1>({size}, &*it);
    ++it;
  }
  return out;
}

} // namespace Linx

#include "Linx/Data/impl/TilingGenerator.h"

#endif
