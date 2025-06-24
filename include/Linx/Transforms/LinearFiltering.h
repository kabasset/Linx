// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_LINEARFILTERING_H
#define LINX_TRANSFORMS_LINEARFILTERING_H

#include "Linx/Data/Image.h"
#include "Linx/Data/Map.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Transforms/mixins/FilterMixin.h"

#include <concepts>
#include <string>

namespace Linx {

/**
 * @ingroup filtering
 * @brief A spatial filter which computes the sum of the elements in the footprint.
 */
template <typename TFootprint>
struct SumFilter : public SpatialFilterMixin<TFootprint, SumFilter<TFootprint>> {
public:

  SumFilter(TFootprint footprint) : SpatialFilterMixin<TFootprint, SumFilter>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "SumFilter";
  }

  template <typename TIn>
  class Lazy : public LazySpatialFilterMixin<SumFilter, TIn, Lazy<TIn>> {
  public:

    using value_type = const typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    using LazySpatialFilterMixin<SumFilter, TIn, Lazy>::LazySpatialFilterMixin;

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      element_type out {};
      for (const auto& e : neighbors) {
        out += e;
      }
      return out;
    }
  };
};

/**
 * @ingroup filtering
 * @brief A spatial filter that computes the mean of the elements in the footprint.
 */
template <typename TFootprint>
class MeanFilter : public SpatialFilterMixin<TFootprint, MeanFilter<TFootprint>> {
public:

  MeanFilter(TFootprint footprint) : SpatialFilterMixin<TFootprint, MeanFilter>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "MeanFilter";
  }

  template <typename TIn>
  class Lazy : public LazySpatialFilterMixin<MeanFilter, TIn, Lazy<TIn>> {
  public:

    using value_type = const typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    using LazySpatialFilterMixin<MeanFilter, TIn, Lazy>::LazySpatialFilterMixin;

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      element_type out {};
      for (const auto& e : neighbors) {
        out += e;
      }
      return out / this->m_offsets.size();
    }
  };
};

namespace Impl {

struct Conjugate {
  KOKKOS_INLINE_FUNCTION auto operator()(const auto& e) const
  {
    return Kokkos::conj(e);
  }
};

} // namespace Impl

/**
 * @ingroup filtering
 * @brief A correlation filter.
 * 
 * A correlation filter is a linear filter that applies a weighted sum of the input elements to produce an output element.
 * The weights are given by the kernel, which can be sparse (e.g. using a `Map`).
 * If the element type is complex, the kernel is conjugated before applying the filter.
 * 
 * @see `Convolution`
 */
template <typename TKernel>
class Correlation : public WeightedFilterMixin<TKernel, Correlation<TKernel>> {
public:

  using value_type = const typename TKernel::value_type;
  using element_type = std::remove_cvref_t<value_type>;

  Correlation(TKernel kernel) : WeightedFilterMixin<TKernel, Correlation>(LINX_MOVE(kernel)) {}

  std::string label() const
  {
    return "Correlation";
  }

  template <typename TIn>
  class Lazy : public LazyWeightedFilterMixin<Correlation, TIn, Lazy<TIn>> {
  public:

    Lazy(Correlation filter, const TIn& in) : LazyWeightedFilterMixin<Correlation, TIn, Lazy>(LINX_MOVE(filter), in)
    {
      if constexpr (is_complex<element_type>()) {
        conjugate_impl();
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

    // Cannot be private
    // Cannot be the ctor (must take address)
    // FIXME free function? Nested Conjugate?
    void conjugate_impl() const
    {
      this->m_weights.transform("conjugate", Impl::Conjugate());
    }
  };
};

/**
 * @ingroup filtering
 * @brief Convolution filter class.
 * 
 * @see `Correlation`
 */
template <typename TKernel>
class Convolution : public WeightedFilterMixin<TKernel, Convolution<TKernel>> {
public:

  using value_type = const typename TKernel::value_type; // FIXME deduce from reduce()
  using element_type = std::remove_cvref_t<value_type>;

  Convolution(TKernel kernel) : WeightedFilterMixin<TKernel, Convolution>(LINX_MOVE(kernel)) {}

  std::string label() const
  {
    return "Convolution";
  }

  template <typename TIn>
  class Lazy : public LazyWeightedFilterMixin<Convolution, TIn, Lazy<TIn>> {
  public:

    Lazy(Convolution filter, const TIn& in) : LazyWeightedFilterMixin<Convolution, TIn, Lazy>(LINX_MOVE(filter), in)
    {
      this->m_weights.reverse(); // TODO use rbegin() in reduce instead?
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

/**
 * @ingroup filtering
 * @brief Non-isotropic, separable Laplacian filter.
 * @param s A scale factor
 * 
 * The kernel is a sum of kernels of the form `{s, -2 * s, s}`.
 * For example, by default (`s = 1`), the 2D kernel is:
 * ```
 *  0  1  0
 *  1 -4  1
 *  0  1  0
 * ```
 * 
 * The filter is implemented as a correlation, such that conjugation is involved when the kernel is complex-valued.
 */
template <std::integral auto... Is, typename T = int> // FIXME T = Forward
auto separable_laplacian(T s = T(1))
{
  static constexpr auto N = std::max({Is...}) + 1;
  auto kernel = Map<T, N>();
  for (auto i : {Is...}) {
    auto p = Position<N>();
    kernel[p] += -2 * s;
    p[i] = -1;
    kernel[p] = s;
    p[i] = 1;
    kernel[p] = s;
  }
  return Correlation(LINX_MOVE(kernel));
}

/**
 * @brief Create a 1D sampled Gaussian kernel.
 * 
 * Example 2D Gaussian filtering of an image with 0-padding:
 * 
 * ```
 * auto sigma = 3.0;
 * auto kernel = Linx::sampled_gaussian_kernel(sigma, 3 * sigma);
 * auto filter = Linx::convolution_along<0, 1>(kernel);
 * auto out = filter.pad(0)(in);
 * ```
 */
template <typename T>
Shift<Sequence<T, -1>> sampled_gaussian_kernel(const T& sigma, Index radius)
{
  auto kernel = Shift(Sequence<T, -1>("gaussian kernel", 2 * radius + 1), -radius);
  const auto norm = std::numbers::inv_sqrtpi * std::numbers::sqrt2 * 0.5 / sigma;
  const auto factor = -0.5 / (sigma * sigma);
  for_each(
      "Gaussian kernel",
      kernel.domain(),
      KOKKOS_LAMBDA(int i) { kernel(i) = norm * std::exp(i * i * factor); });
  return kernel;
}

template <Index I, Index N = I + 1> // FIXME rm N, support non matching ranks in FilterMixin
auto convolution_along(auto&& kernel)
{
  return Convolution(along<I, N>(on_host(LINX_FORWARD(kernel)))); // FIXME on_host() in WeightedFilterMixin
}

template <Index I, Index N = I + 1> // FIXME rm N, support non matching ranks in FilterMixin
auto correlation_along(auto&& kernel)
{
  return Correlation(along<I, N>(on_host(LINX_FORWARD(kernel)))); // FIXME on_host() in WeightedFilterMixin
}

} // namespace Linx

#endif
