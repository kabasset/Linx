// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_CORRELATION_H // FIXME rename as LinearFiltering
#define LINX_TRANSFORMS_CORRELATION_H

#include "Linx/Data/Image.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Transforms/mixins/FilterMixin.h"

#include <Kokkos_StdAlgorithms.hpp>
#include <concepts>
#include <string>

namespace Linx {

template <typename TFootprint>
struct SumFilter : public SpatialFilterMixin<TFootprint, SumFilter<TFootprint>> {
public:

  SumFilter(TFootprint footprint) : SpatialFilterMixin<TFootprint, SumFilter>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "SumFilter";
  }

  template <typename TIn>
  class Apply : public ApplySpatialFilterMixin<SumFilter, TIn, Apply<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    using ApplySpatialFilterMixin<SumFilter, TIn, Apply>::ApplySpatialFilterMixin;

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      return std::reduce(neighbors.begin(), neighbors.end());
    }
  };
};

template <typename TFootprint>
class MeanFilter : public SpatialFilterMixin<TFootprint, MeanFilter<TFootprint>> {
public:

  MeanFilter(TFootprint footprint) : SpatialFilterMixin<TFootprint, MeanFilter>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "MeanFilter";
  }

  template <typename TIn>
  class Apply : public ApplySpatialFilterMixin<MeanFilter, TIn, Apply<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    using ApplySpatialFilterMixin<MeanFilter, TIn, Apply>::ApplySpatialFilterMixin;

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      return std::reduce(neighbors.begin(), neighbors.end()) / this->m_offsets.size();
    }
  };
};

template <typename TKernel>
class Correlation : public WeightedFilterMixin<TKernel, Correlation<TKernel>> {
public:

  using value_type = typename TKernel::value_type;
  using element_type = std::remove_cvref_t<value_type>;

  Correlation(TKernel kernel) : WeightedFilterMixin<TKernel, Correlation>(LINX_MOVE(kernel)) {}

  std::string label() const
  {
    return "Correlation";
  }

  template <typename TIn>
  class Apply : public ApplyWeightedFilterMixin<Correlation, TIn, Apply<TIn>> {
  public:

    Apply(const Correlation& filter, const TIn& in) : ApplyWeightedFilterMixin<Correlation, TIn, Apply>(filter, in)
    {
      if constexpr (is_complex<element_type>()) {
        this->m_weights.apply(
            "conjugate",
            KOKKOS_LAMBDA(auto e) { return Kokkos::conj(e); });
      }
    }

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      element_type out {};
      auto wit = this->m_weights.begin();
      for (auto nit = neighbors.begin(); nit != neighbors.end(); ++nit, ++wit) {
        out += *nit * *wit;
      }
      return out;
    }
  };
};

} // namespace Linx

#endif
