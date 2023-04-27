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
  const auto plane = project(box(in), I);
  Raster<Line<I, TRegion::Dimension>, TRegion::Dimension> out(plane.shape());
  auto frontIt = plane.begin();
  for (auto& e : out) {
    e = Line<I, TRegion::Dimension>::fromSize(*frontIt, size, step[I]);
    ++frontIt;
  }
  return out;
}

template <Index I, typename TRaster>
Raster<Patch<TRaster, Line<I, TRaster::Dimension>>, TRaster::Dimension> tileRasterAlong(const TRaster& in) {
  const auto size = in.template length<I>();
  const auto step = in.step();
  Raster<Patch<TRaster, Line<I, TRaster::Dimension>>, TRaster::Dimension> out(project(in.domain(), I).shape());
  auto frontIt = out.domain().begin();
  for (auto& e : out.domain()) {
    e = in.patch(Line<I, TRaster::Dimension>::fromSize(*frontIt, size, step));
    ++frontIt;
  }
  return out;
}

} // namespace Linx

#endif
