/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXCORE_TILING_H
#define _LINXCORE_TILING_H

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

template <Index I, typename TRaster>
auto tileRasterAlong(const TRaster& in) {
  using T = typename TRaster::Value;
  static constexpr Index N = TRaster::Dimension;
  const auto domain = in.domain();
  const auto size = domain.template length<I>();
  const auto step = domain.step();
  const auto plane = project(domain, I);
  Raster<Patch<const T, const TRaster, Line<I, N>>, N> out(plane.shape());
  auto frontIt = plane.begin();
  for (auto& e : out) {
    e = in.patch(Line<I, N>::fromSize(*frontIt, size, step[I]));
    ++frontIt;
  }
  return out;
}

} // namespace Linx

#endif
