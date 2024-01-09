/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_TILING_H
#define _LINXDATA_TILING_H

#include "Linx/Data/Grid.h"
#include "Linx/Data/Raster.h"

namespace Linx {

/// @cond
namespace Internal {

/**
 * @ingroup regions
 * @brief Iterator over tiles of a raster.
 * @see tiles()
 */
template <typename TRaster, Index M>
class TileGenerator;

/**
 * @ingroup regions
 * @brief Iterator over the sections of a raster.
 * @see sections()
 */
template <typename TRaster>
class SectionGenerator;

/**
 * @ingroup regions
 * @brief Iterator over multi-sections of a raster.
 * @see sections()
 */
template <typename TRaster>
class MultisectionGenerator;

/**
 * @ingroup regions
 * @brief Iterator over the `I`-th axis-aligned profiles of a raster.
 */
template <Index I, typename TRaster>
class ProfileGenerator;

/**
 * @ingroup regions
 * @brief Iterator over the rows of a raster.
 */
template <typename TRaster>
class RowGenerator;

} // namespace Internal
/// @endcond

/**
 * @ingroup regions
 * @brief Get a box-patch partitioning of a raster.
 * 
 * The input raster domain is partitioned into boxes of given shape,
 * except at its upper limits where the boxes may be clamped inside the input domain.
 * A range over the corresponding patches is returned.
 */
template <typename TRaster, Index M>
auto tiles(TRaster& in, Position<M> shape)
{
  return Internal::TileGenerator<TRaster, M>(in, std::move(shape));
}

/**
 * @ingroup regions
 * @brief Make a raster of tiles.
 */
template <typename TParent, Index M>
auto rasterize(const Internal::TileGenerator<TParent, M>& generator)
{
  return generator.raster();
}

/**
 * @ingroup regions
 * @brief Get a slicing of a raster as a range of cross-sections of given thickness.
 * 
 * The input raster domain is sliced along its last axis into sections of given thickness,
 * except for the last section which may be thinner.
 * A range over the sections is returned.
 */
template <typename TRaster>
auto sections(TRaster& in, Index thickness = 1) // FIXME overload for thickness = 1, with dimension N-1
{
  return Internal::MultisectionGenerator<TRaster>(in, thickness);
}

/**
 * @ingroup regions
 * @brief Make a raster of sections.
 */
template <typename TParent>
auto rasterize(const Internal::SectionGenerator<TParent>& generator)
{
  return generator.raster();
}

/**
 * @ingroup regions
 * @brief Get a line-patch partitioning of a raster.
 * @tparam I The index of the axis
 * 
 * The input raster domain is partitioned into maximal lines oriented along the `I`-th axis.
 * A range over the corresponding patches is returned.
 */
template <Index I, typename TRaster>
auto profiles(TRaster& in)
{
  using TPatch = std::decay_t<decltype(in(Line<I, TRaster::Dimension>()))>;
  const auto domain = in.domain();
  const auto size = domain.template length<I>();
  const auto step = domain.step();
  const auto plane = project(domain, I);
  Raster<TPatch, TRaster::Dimension> out(plane.shape());
  auto front_it = plane.begin();
  for (auto& e : out) {
    e = in(Line<I, TRaster::Dimension>::from_size(*front_it, size, step[I]));
    ++front_it;
  }
  return out;
}

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
auto rows(TRaster& in)
{
  return Internal::RowGenerator<TRaster>(in);
}

/**
 * @ingroup regions
 * @brief Make a raster of rows.
 */
template <typename TParent>
auto rasterize(const Internal::RowGenerator<TParent>& generator)
{
  return generator.raster();
}

} // namespace Linx

#include "Linx/Data/impl/RowGenerator.h"
#include "Linx/Data/impl/SectionGenerator.h"
#include "Linx/Data/impl/TileGenerator.h"

#endif
