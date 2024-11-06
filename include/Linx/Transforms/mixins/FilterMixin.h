// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_FILTERMIXIN_H
#define LINX_TRANSFORMS_FILTERMIXIN_H

#include "Linx/Base/ArrayPool.h"
#include "Linx/Data/Sequence.h"

#include <string>

namespace Linx {

/**
 * @brief Filtering task mixin.
 * 
 * With:
 * 
 * \code
 * class Convolve : public WeightedFilterMixin;
 * class Erode : public FilterMixin;
 * using Opening = Erode * Dilate
 * using Laplacian<0, 1> = Laplacian<0> + Laplacian<1>;
 * \endcode
 * 
 * Possible usage:
 * 
 * \code
 * auto cropped = Linx::Convolve(kernel)(in);
 * auto [cropped] = P::Run("convolution") | in | Linx::Convolve(kernel);
 * auto extrapolated = Linx::Convolve(kernel).pad(0)(in);
 * auto extrapolated = Linx::Convolve(kernel)(Linx::Pad(0)(in));
 * auto extrapolated = Linx::Convolve(kernel)(Linx::NearestNeighborExtrapolation()(in));
 * auto extrapolated = Linx::Convolve(kernel)(Linx::NearestNeighborExtrapolation()(in));
 * auto [extrapolated] = P::Run("convolution") | in | Linx::Pad(0) | Linx::Convolve(kernel);
 * auto [extrapolated] = P::Run("convolution") | in | Linx::Convolve(kernel).pad(0);
 * auto [extrapolated] = P::Run("convolution") | in | Linx::NearestNeighborExtrapolation() | Linx::Convolve(kernel);
 * 
 * auto cropped = Linx::Erode(radius)(in);
 * auto [cropped] = P::Run("erosion") | in | Linx::Erode(radius);
 * auto extrapolated = Linx::Erode(radius)(Linx::Pad()(in)); // Deduce padding
 * auto [extrapolated] = P::Run("erosion") | in | Linx::WithExtrapolation() | Linx::Erode(radius);
 * 
 * auto cropped = Linx::Erode(radius) * Linx::Dilate(radius) * in;
 * auto [cropped] = P::Run("opening") | in | Linx::Erode(radius) * Linx::Dilate(radius);
 * auto extrapolated = Linx::Erode(radius) * Linx::Dilate(radius) * Linx::Pad() * in;
 * auto [extrapolated] = P::Run("opening") | in | Linx::Pad() | Linx::Erode(radius) * Linx::Dilate(radius);
 * auto [extrapolated] = P::Run("opening") | in | Linx::Pad(Linx::Erode(radius) * Linx::Dilate(radius));
 * auto [extrapolated] = P::Run("opening") | in | Linx::Erode(radius).pad() * Linx::Dilate(radius).pad();
 * \endcode
 */
template <typename TFootprint, typename TPolicy, typename TDerived>
class FilterMixin {
public:

  FilterMixin(TFootprint&& footprint, TPolicy&& policy) :
      m_footprint(LINX_FORWARD(footprint)), m_policy(LINX_FORWARD(policy))
  {}

  auto operator()(const auto& in)
  {
    return Filtered(LINX_CRTP_CONST_DERIVED, in);
  }

private:

  TFootprint m_footprint; ///< The footprint
  TPolicy m_policy; ///< The border policy
};

template <typename TIn, typename TDerived>
class MorphologyFilterMixin { // FIXME simply FilterMixin?
public:

  MorphologyFilterMixin(const auto& strel, const TIn& in) :
      MorphologyFilterMixin(Sequence<std::ptrdiff_t, -1>("offsets", strel.size()), in)
  {
    auto offsets_on_host = on_host(m_offsets);
    auto index = std::make_shared<Index>(0);
    auto front = &m_in.front();
    for_each<Kokkos::Serial>(
        "compute_offsets()", // FIXME analytic through strides?
        strel,
        KOKKOS_LAMBDA(std::integral auto... is) {
          offsets_on_host[*index] = &m_in(is...) - front;
          ++(*index);
        });
    copy_to(offsets_on_host, m_offsets); // FIXME offsets_on_host.copy_to(m_offsets)
  }

protected:

  MorphologyFilterMixin(const Sequence<std::ptrdiff_t, -1>& offsets, const TIn& in) :
      m_offsets(offsets), m_in(as_readonly(in))
  {}

  Sequence<std::ptrdiff_t, -1> m_offsets;
  decltype(as_readonly(std::declval<TIn>())) m_in;
};

template <typename TIn, typename TDerived>
const TDerived& as_readonly(const MorphologyFilterMixin<TIn, TDerived>& in)
{
  return static_cast<const TDerived&>(in);
}

template <typename TKernel, typename TIn, typename TDerived>
class WeightedFilterMixin : public MorphologyFilterMixin<TIn, TDerived> {
public:

  WeightedFilterMixin(const auto& kernel, const auto& in) :
      MorphologyFilterMixin<TIn, TDerived>(Sequence<std::ptrdiff_t, -1>("offsets", kernel.size()), in),
      m_weights("weights", kernel.size())
  {
    // FIXME delegate m_offsets computation to Morphology?

    auto offsets_on_host = on_host(this->m_offsets);
    auto weights_on_host = on_host(m_weights);
    auto index = std::make_shared<Index>(0);
    auto front = &this->m_in.front();
    for_each<Kokkos::Serial>(
        "compute_offsets()",
        kernel.domain(),
        KOKKOS_LAMBDA(std::integral auto... is) {
          offsets_on_host[*index] = &this->m_in(is...) - front;
          weights_on_host[*index] = kernel(is...);
          ++(*index);
        });
    copy_to(offsets_on_host, this->m_offsets); // FIXME offsets_on_host.copy_to(m_offsets)
    copy_to(weights_on_host, m_weights); // FIXME
  }

protected:

  Sequence<typename TKernel::element_type, -1> m_weights;
};

} // namespace Linx

#endif
