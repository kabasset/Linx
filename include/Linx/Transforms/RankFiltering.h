// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_RANKFILTERING_H
#define LINX_TRANSFORMS_RANKFILTERING_H

#include "Linx/Base/Algorithm.h"
#include "Linx/Base/ArrayPool.h"
#include "Linx/Base/SelectNet.h"
#include "Linx/Data/Image.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Transforms/mixins/FilterMixin.h"

#include <concepts>
#include <string>

namespace Linx {

template <int Size, typename TFootprint>
class MedianFilter : public SpatialFilterMixin<TFootprint, MedianFilter<Size, TFootprint>> {
public:

  MedianFilter(TFootprint footprint) : SpatialFilterMixin<TFootprint, MedianFilter>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "MedianFilter";
  }

  template <typename TIn>
  class Apply : public ApplySpatialFilterMixin<MedianFilter, TIn, Apply<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    Apply(const MedianFilter& filter, TIn in) : ApplySpatialFilterMixin<MedianFilter, TIn, Apply>(filter, LINX_MOVE(in))
    {}

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      Kokkos::Array<element_type, Size> array;
      for (std::size_t i = 0; i < std::size(array); ++i) {
        array[i] = neighbors[i];
      }
      return SelectNet<Size>::median(array); // FIXLE is it any faster than median()???
    }
  };
};

template <typename TFootprint>
class MedianFilter<0, TFootprint> : public SpatialFilterMixin<TFootprint, MedianFilter<0, TFootprint>> {
public:

  MedianFilter(TFootprint footprint) : SpatialFilterMixin<TFootprint, MedianFilter>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "MedianFilter";
  }

  template <typename TIn>
  class Apply : public ApplySpatialFilterMixin<MedianFilter, TIn, Apply<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    Apply(const MedianFilter& filter, TIn in) :
        ApplySpatialFilterMixin<MedianFilter, TIn, Apply>(filter, LINX_MOVE(in)),
        m_neighbors(this->m_offsets.size())
    {}

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      auto array = m_neighbors.array();
      for (std::size_t i = 0; i < std::size(array); ++i) {
        array[i] = neighbors[i];
      }
      return median(array);
    }

  private:

    ArrayPool<element_type> m_neighbors; // FIXME TSpace
  };
};

template <typename T>
MedianFilter(const T&) -> MedianFilter<0, T>;

/**
 * @brief Minimum filter, aka. grayscale erosion.
 */
template <typename TFootprint>
class MinimumFilter : public SpatialFilterMixin<TFootprint, MinimumFilter<TFootprint>> {
public:

  MinimumFilter(TFootprint footprint) : SpatialFilterMixin<TFootprint, MinimumFilter>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "MinimumFilter";
  }

  template <typename TIn>
  class Apply : public ApplySpatialFilterMixin<MinimumFilter, TIn, Apply<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    using ApplySpatialFilterMixin<MinimumFilter, TIn, Apply>::ApplySpatialFilterMixin;

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      element_type out = neighbors[0];
      for (std::size_t i = 1; i < neighbors.size(); ++i) {
        out = neighbors[i] < out ? neighbors[i] : out;
      }
      return out;
    }
  };
};

/**
 * @brief Maximum filter, aka. grayscale erosion.
 */
template <typename TFootprint>
class MaximumFilter : public SpatialFilterMixin<TFootprint, MaximumFilter<TFootprint>> {
public:

  MaximumFilter(TFootprint footprint) : SpatialFilterMixin<TFootprint, MaximumFilter>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "MaximumFilter";
  }

  template <typename TIn>
  class Apply : public ApplySpatialFilterMixin<MaximumFilter, TIn, Apply<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    using ApplySpatialFilterMixin<MaximumFilter, TIn, Apply>::ApplySpatialFilterMixin;

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      element_type out = neighbors[0];
      for (std::size_t i = 1; i < neighbors.size(); ++i) {
        out = neighbors[i] > out ? neighbors[i] : out;
      }
      return out;
    }
  };
};

} // namespace Linx

#endif
