// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_LINEARFILTERING_H
#define LINX_TRANSFORMS_LINEARFILTERING_H

#include "Linx/Transforms/mixins/Filter.h"

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

    Lazy(MeanFilter filter, const TIn& in) :
        LazySpatialFilterMixin<MeanFilter, TIn, Lazy>(LINX_MOVE(filter), in),
        m_size(this->m_neighbors.size())
    {}

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      element_type out {};
      for (const auto& e : neighbors) {
        out += e;
      }
      return out / m_size;
    }

  private:

    std::size_t m_size;
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
      auto wit = begin(this->m_weights);
      for (auto nit = neighbors.begin(); nit != neighbors.end(); ++nit, ++wit) {
        out += *nit * *wit;
      }
      return out;
    }

    // Cannot be private
    // Cannot be the ctor (must take address)
    void conjugate_impl() const
    {
      this->m_weights.transform("conjugate", Impl::Conjugate()); // FIXME this->m_weights.conj()
    }
  };
};

/**
 * @ingroup filtering
 * @brief Convolution filter class.
 * 
 * As opposed to correlation, the convolution kernel is inversed wrt. the origin,
 * such that the impulse response is exactly the kernel,
 * and convolution is associative and commutative.
 * Consequently, the filter footprint is the opposite of the kernel domain.
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

  auto footprint() const
  {
    return -WeightedFilterMixin<TKernel, Convolution>::footprint();
  }

  template <typename TIn>
  class Lazy : public LazyWeightedFilterMixin<Convolution, TIn, Lazy<TIn>> {
  public:

    Lazy(Convolution filter, const TIn& in) : LazyWeightedFilterMixin<Convolution, TIn, Lazy>(LINX_MOVE(filter), in)
    {
      this->m_neighbors.inverse();
    }

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      element_type out {};
      auto wit = begin(this->m_weights);
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
template <std::integral auto... Is, typename TSpace = Kokkos::DefaultExecutionSpace, typename T = int>
auto separable_laplacian(T s = T(1))
{
  static constexpr auto n = std::max({Is...}) + 1;
  auto kernel = fill<TSpace>("kernel", T {0}, cube<Dimension {n}, 1>()); // FIXME map?
  const auto& kernel_on_host = on_host(kernel);
  for (auto i : {Is...}) {
    auto p = vec<Dimension {n}>(0);
    kernel_on_host.at(p) += -2 * s;
    p[i] = -1;
    kernel_on_host.at(p) = s;
    p[i] = 1;
    kernel_on_host.at(p) = s;
  }
  Kokkos::deep_copy(kernel.container(), kernel_on_host.container());
  return Correlation(LINX_MOVE(kernel));
}

/**
 * @brief AD Gaussian function.
 */
template <typename T>
struct Gaussian {
  /**
   * @brief Constructor.
   */
  constexpr Gaussian(T sigma) :
      m_norm(std::numbers::inv_sqrtpi * std::numbers::sqrt2 * 0.5 / sigma),
      m_constant(-0.5 / (sigma * sigma))
  {}

  /**
   * @brief Call operator.
   */
  KOKKOS_INLINE_FUNCTION constexpr T operator()(std::integral auto i) const
  {
    return m_norm * std::exp(i * i * m_constant);
  }

  T m_norm; ///< The normalization factor
  T m_constant; ///< The constant factor in the exponential
};

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
auto sampled_gaussian_kernel(const auto& sigma, Index radius)
{
  using namespace Linx::Literals;
  return generate("gaussian kernel", Gaussian(sigma), cube<1_D>(radius));
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
