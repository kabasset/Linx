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
template <typename TParent, Index M>
class TileGenerator;

/**
 * @ingroup regions
 * @brief Iterator over the sections of a raster.
 * @see sections()
 */
template <typename TParent>
class SectionGenerator;

/**
 * @ingroup regions
 * @brief Iterator over multi-sections of a raster.
 * @see sections()
 */
template <typename TParent>
class MultisectionGenerator;

/**
 * @ingroup regions
 * @brief Iterator over the `I`-th axis-aligned profiles of a raster.
 */
template <typename TParent, Index I>
class ProfileGenerator;

/**
 * @ingroup regions
 * @brief Iterator over the rows of a raster.
 */
template <typename TParent>
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
template <typename TParent, Index M>
auto tiles(TParent& in, Position<M> shape)
{
  return Internal::TileGenerator<TParent, M>(in, std::move(shape));
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
template <typename TParent>
auto sections(TParent& in, Index thickness)
{
  return Internal::MultisectionGenerator<TParent>(in, thickness);
}

/**
 * @ingroup regions
 * @brief Get a slicing of a raster as a range of cross-sections of given thickness.
 * 
 * The input raster domain is sliced along its last axis into sections of given thickness,
 * except for the last section which may be thinner.
 * A range over the sections is returned.
 */
template <typename TParent>
auto sections(TParent& in)
{
  return Internal::SectionGenerator<TParent>(in);
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
template <Index I, typename TParent>
auto profiles(TParent& in)
{
  return Internal::ProfileGenerator<TParent, I>(in);
}

/**
 * @ingroup regions
 * @brief Make a raster of profiles.
 */
template <typename TParent, Index I>
auto rasterize(const Internal::ProfileGenerator<TParent, I>& generator)
{
  return generator.raster();
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
template <typename TParent>
auto rows(TParent& in)
{
  return Internal::RowGenerator<TParent>(in);
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

#include "Linx/Data/impl/ProfileGenerator.h"
#include "Linx/Data/impl/RowGenerator.h"
#include "Linx/Data/impl/SectionGenerator.h"
#include "Linx/Data/impl/TileGenerator.h"

#endif
