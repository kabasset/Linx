// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_IMPL_REGION_HPP
#define _LINXDATA_IMPL_REGION_HPP

#include "Linx/Data/Vector.h"

#ifdef DOXYGEN

/**
 * @brief The Linx concepts.
 */
namespace Concept {

/**
 * @ingroup concepts
 * @requirements{Region}
 * @brief An iterable of positions.
 * 
 * Any iterable of positions is a valid region, but functions can be specialized for optimization purpose.
 */
template <Index N>
struct Region {
  /**
   * @brief Move the region by a given vector.
   */
  Region& operator+=(Position<N>&);

  /**
   * @brief Move the region by the opposite of a given vector.
   */
  Region& operator-=(Position<N>&);

  /**
   * @brief Clamp the region by a box.
   */
  Region& operator&=(Box<N>&);
};

/**
 * @relatesalso Region
 * @brief Get the bounding box of the region.
 */
template <Index N>
Box<N> box(const Region&);

} // namespace Concept

#endif

namespace Linx {

/// @cond
template <Index N>
class Box; // for box
/// @endcond

/**
 * @ingroup mixins
 * @brief Add region operations to an iterable of positions.
 * 
 * The derived class must implement the following operators:
 * 
 * - `operator+=(const Position<M>&)`
 * - `operator-=(const Position<M>&)`
 * - `operator&=(const Box<N>&)`
 */
template <Index N, typename TDerived>
class RegionMixin {
public:

  /**
   * @brief The region dimension.
   */
  static constexpr Index Dimension = N;
};

/**
 * @relatesalso Region
 * @brief Get the bounding box of a region.
 * 
 * This generic implementation is unoptimized:
 * it iterates over all of the positions.
 */
template <typename TIn>
Box<TIn::Dimension> box(const TIn& region)
{ // FIXME enable if is_region<TIn>()
  const auto dim = region.dimension();
  auto front = Position<TIn::Dimension>(dim).fill(std::numeric_limits<Index>::max());
  auto back = Position<TIn::Dimension>(dim).fill(std::numeric_limits<Index>::min());
  for (const auto& p : region) {
    for (Index i = 0; i < dim; ++i) {
      front[i] = std::min(front[i], p[i]);
      back[i] = std::max(back[i], p[i]);
    }
  }
  return {front, back};
}

} // namespace Linx

#endif