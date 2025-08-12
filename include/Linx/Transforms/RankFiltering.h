// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_RANKFILTERING_H
#define LINX_TRANSFORMS_RANKFILTERING_H

#include "Linx/Base/ArrayPool.h"
#include "Linx/Base/Median.h"
#include "Linx/Transforms/mixins/Filter.h"

#include <concepts>
#include <string>

namespace Linx {

/**
 * @ingroup filtering
 * @brief Median filter.
 */
template <typename TFootprint>
class MedianFilter : public SpatialFilterMixin<TFootprint, MedianFilter<TFootprint>> {
public:

  static constexpr bool static_size_flag = TFootprint::static_flag;

  MedianFilter(TFootprint footprint) : SpatialFilterMixin<TFootprint, MedianFilter>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "MedianFilter";
  }

  template <typename TIn>
  class StaticSizeLazy : public LazySpatialFilterMixin<MedianFilter, TIn, StaticSizeLazy<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    StaticSizeLazy(const MedianFilter& filter, TIn in) :
        LazySpatialFilterMixin<MedianFilter, TIn, StaticSizeLazy>(filter, LINX_MOVE(in))
    {}

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      static constexpr auto n = TFootprint().size();
      Kokkos::Array<element_type, n> array;
      for (std::size_t i = 0; i < std::size(array); ++i) {
        array[i] = neighbors[i];
      }
      return median<n>(array);
    }
  };

  template <typename TIn>
  class DynamicSizeLazy : public LazySpatialFilterMixin<MedianFilter, TIn, DynamicSizeLazy<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    DynamicSizeLazy(const MedianFilter& filter, TIn in) :
        LazySpatialFilterMixin<MedianFilter, TIn, DynamicSizeLazy>(filter, LINX_MOVE(in)),
        m_buffer(this->m_neighbors.size())
    {}

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      auto array = m_buffer.array();
      for (std::size_t i = 0; i < std::size(array); ++i) {
        array[i] = neighbors[i];
      }
      return median(array);
    }

  private:

    ArrayPool<element_type> m_buffer; // FIXME TSpace
  };

  template <typename TIn>
  using Lazy = std::conditional_t<static_size_flag, StaticSizeLazy<TIn>, DynamicSizeLazy<TIn>>;
};

/**
 * @ingroup filtering
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
  class Lazy : public LazySpatialFilterMixin<MinimumFilter, TIn, Lazy<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    using LazySpatialFilterMixin<MinimumFilter, TIn, Lazy>::LazySpatialFilterMixin;

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      element_type out = neighbors[0];
      for (int i = 1; i < neighbors.ssize(); ++i) {
        out = neighbors[i] < out ? neighbors[i] : out;
      }
      return out;
    }
  };
};

/**
 * @ingroup filtering
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
  class Lazy : public LazySpatialFilterMixin<MaximumFilter, TIn, Lazy<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    using LazySpatialFilterMixin<MaximumFilter, TIn, Lazy>::LazySpatialFilterMixin;

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      element_type out = neighbors[0];
      for (int i = 1; i < neighbors.ssize(); ++i) {
        out = neighbors[i] > out ? neighbors[i] : out;
      }
      return out;
    }
  };
};

} // namespace Linx

#endif
