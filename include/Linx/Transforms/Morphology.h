// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_MORPHOLOGY_H
#define LINX_TRANSFORMS_MORPHOLOGY_H

#include "Linx/Base/Algorithm.h"
#include "Linx/Base/ArrayPool.h"
#include "Linx/Data/Image.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Transforms/mixins/FilterMixin.h"

#include <concepts>
#include <string>

namespace Linx {

/**
 * @brief Binary erosion.
 */
template <typename TFootprint>
class Erosion : public SpatialFilterMixin<TFootprint, Erosion<TFootprint>> {
public:

  Erosion(TFootprint footprint) : SpatialFilterMixin<TFootprint, Erosion>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "Erosion";
  }

  template <typename TIn>
  class Apply : public ApplySpatialFilterMixin<TFootprint, TIn, Apply<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    Apply(TFootprint footprint, TIn in) :
        ApplySpatialFilterMixin<TFootprint, TIn, Apply>(LINX_MOVE(footprint), LINX_MOVE(in))
    {}

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      return std::all_of(neighbors.begin(), neighbors.end(), [](auto e) {
        return bool(e);
      });
    }
  };
};

/**
 * @brief Binary dilation.
 */
template <typename TFootprint>
class Dilation : public SpatialFilterMixin<TFootprint, Dilation<TFootprint>> {
public:

  Dilation(TFootprint footprint) : SpatialFilterMixin<TFootprint, Dilation>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "Dilation";
  }

  template <typename TIn>
  class Apply : public ApplySpatialFilterMixin<TFootprint, TIn, Apply<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    Apply(TFootprint footprint, TIn in) :
        ApplySpatialFilterMixin<TFootprint, TIn, Apply>(LINX_MOVE(footprint), LINX_MOVE(in))
    {}

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      return std::any_of(neighbors.begin(), neighbors.end(), [](auto e) {
        return bool(e);
      });
    }
  };
};

} // namespace Linx

#endif
