/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXCORE_TILING_H
#define _LINXCORE_TILING_H

#include "LinxCore/Grid.h"
#include "LinxCore/Raster.h"

namespace Linx {

template <Index I, typename TRegion>
Raster<Line<I, TRegion::Dimension>, TRegion::Dimension> tileRegionAlong(const TRegion& in) {
  const auto size = in.template length<I>();
  const auto step = in.step();
  const auto plane = project(in, I);
  Raster<Line<I, TRegion::Dimension>, TRegion::Dimension> out(plane.shape());
  auto frontIt = plane.begin();
  for (auto& e : out) {
    e = Line<I, TRegion::Dimension>::fromSize(*frontIt, size, step[I]);
    ++frontIt;
  }
  return out;
}

/**
 * @ingroup regions
 * @brief Get a line-patch partitioning of a raster.
 * 
 * The input raster domain is partitioned into maximal lines oriented along the `I`-th axis.
 * An iterable over the corresponding patches is returned.
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
 * @brief Get a line-section partitioning of a raste.
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

/**
 * @ingroup regions
 * @brief Get a box-patch partitioning of a raster.
 * 
 * The input raster domain is partitioned into boxes of given shape,
 * except at its upper limits where the boxes may be clamped inside the input domain.
 * An iterable over the corresponding patches is returned.
 */
template <typename TRaster, Index M>
auto tiles(TRaster& in, const Position<M>& shape) {
  using TPatch = std::decay_t<decltype(in.patch(Box<M>()))>;
  const auto domain = in.domain();
  const Grid<M> fronts(domain, shape);
  Raster<TPatch, TRaster::Dimension> out(fronts.shape());
  auto frontIt = fronts.begin();
  for (auto& e : out) {
    e = in.patch(Box<M>::fromShape(*frontIt, shape) & domain);
    ++frontIt;
  }
  return out;
}

/**
 * @ingroup regions
 * @brief Get a slicing of a raster as an iterable of cross-sections of given thickness.
 * 
 * The input raster domain is sliced along its last axis into sections of given thickness,
 * except for the last section which may be thinner.
 * An iterable over the sections is returned.
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

} // namespace Linx

#endif
