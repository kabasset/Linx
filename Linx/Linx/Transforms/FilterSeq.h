// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_FILTERSEQ_H
#define _LINXTRANSFORMS_FILTERSEQ_H

#include "Linx/Base/SeqUtils.h"
#include "Linx/Data/Raster.h"
#include "Linx/Transforms/Extrapolation.h"
#include "Linx/Transforms/Kernel.h"
#include "Linx/Transforms/OrientedKernel.h"

#include <type_traits> // decay

namespace Linx {

/**
 * @brief A sequence of filters.
 */
template <typename... TFilters>
class FilterSeq :
    public FilterMixin<
        typename std::decay_t<decltype((std::declval<TFilters>(), ...))>::Value,
        Box<std::max({TFilters::Dimension...})>,
        FilterSeq<TFilters...>> {
  template <typename... UFilters>
  friend class FilterSeq; // FIXME useful?

public:

  /**
   * @brief The last filter value type.
   */
  using Value = typename std::decay_t<decltype((std::declval<TFilters>(), ...))>::Value;

  /**
   * @brief The logical dimension of the composed kernel.
   */
  static constexpr Index Dimension = std::max({TFilters::Dimension...});

  /**
   * @brief Bring to scope FilterMixin::operator*.
   */
  using FilterMixin<
      typename std::decay_t<decltype((std::declval<TFilters>(), ...))>::Value,
      Box<std::max({TFilters::Dimension...})>,
      FilterSeq<TFilters...>>::operator*;

  /**
   * @brief Constructor.
   * 
   * @note Filter sequences are better constructed by multiplying simple filters.
   */
  template <typename... TArgs>
  explicit FilterSeq(TArgs&&... args) : m_filters(std::forward<TArgs>(args)...)
  {}

  /**
   * @brief Combine two sequences of kernels.
   */
  template <typename... UFilters>
  FilterSeq<TFilters..., UFilters...> operator*(const FilterSeq<UFilters...>& rhs) const
  {
    return FilterSeq<TFilters..., UFilters...>(std::tuple_cat(m_filters, rhs.m_filters));
  }

  /**
   * @brief Append a filter to the sequence.
   */
  template <typename T, typename TWindow>
  FilterSeq<TFilters..., Filter<T, TWindow>> operator*(Filter<T, TWindow>&& rhs) const
  {
    return FilterSeq<TFilters..., Filter<T, TWindow>>(std::tuple_cat(m_filters, std::forward<Filter<T, TWindow>>(rhs)));
  }

  /**
   * @brief Prepend a filter to the sequence.
   */
  template <typename T, typename TWindow>
  friend FilterSeq<Filter<T, TWindow>, TFilters...>
  operator*(Filter<T, TWindow>&& lhs, const FilterSeq<TFilters...>& rhs)
  {
    return FilterSeq<Filter<T, TWindow>, TFilters...>(
        std::tuple_cat(std::make_tuple(std::forward<Filter<T, TWindow>>(lhs)), rhs.m_filters));
  }

  /// @group_properties

  /**
   * @brief The logical window of the kernel.
   */
  Box<Dimension> window() const
  {
    // FIXME support Dimension = -1
    auto front = Position<Dimension>::zero();
    auto back = Position<Dimension>::zero();
    seq_foreach(m_filters, [&](const auto& k) {
      const auto& w = box(k.window());
      for (Index i = 0; i < w.dimension(); ++i) {
        // FIXME sum radius instead of min/max
        front[i] = std::min(front[i], w.front()[i]);
        back[i] = std::max(back[i], w.back()[i]);
      }
    });
    return {front, back};
  }

  /**
   * @brief Get the i-th filter.
   */
  template <Index I>
  const auto& filter() const
  {
    return std::get<I>(m_filters);
  }

  /**
   * @brief Filter an input raster.
   * 
   * The input is cropped according to the filter window just enough so no extrapolation is required.
   */
  template <typename T, Index N, typename THolder, typename TOut>
  void transform(const Raster<T, N, THolder>& in, TOut& out) const
  {
    const auto outK = upto_kth<sizeof...(TFilters) - 2>(in);
    filter<sizeof...(TFilters) - 1>().transform(outK, out);
  }

  /**
   * @brief Filter an input extrapolated raster.
   * 
   * The input and output must have the same size, although not necessarily the same domain.
   */
  template <typename TRaster, typename TMethod, typename TOut>
  void transform(const Extrapolation<TRaster, TMethod>& in, TOut& out) const
  {
    const auto domain0 = in.domain() + extend<TRaster::Dimension>(window());
    const auto outK = upto_kth<sizeof...(TFilters) - 2>(in.patch(domain0));
    filter<sizeof...(TFilters) - 1>().transform(outK, out);
  }

  /**
   * @brief Filter an input patch.
   */
  template <typename T, typename TParent, typename TRegion, typename TOut>
  void transform(const Patch<T, TParent, TRegion>& in, TOut& out) const
  {
    const auto& raw = raster(in);
    const auto& domain = in.domain();
    const auto& extrapolate = in.method();
    static constexpr Index N = sizeof...(TFilters);
    const auto domain0 = box(domain) + extend<TParent::Dimension>(window());
    auto outK = upto_kth<N - 2>(extrapolate(raw.patch(domain0)));
    const auto domainK = in.domain() + filter<N - 1>().origin();
    filter<N - 1>().transform(outK.patch(domainK), out);
  }

private:

  template <std::size_t K, typename TIn>
  auto upto_kth(const TIn& in) const
  {
    const auto& domain = in.domain() - extend<TIn::Dimension>(Linx::box(filter<K>().window()));
    const auto patch = in.patch(domain);
    if constexpr (K == 0) {
      return filter<0>() * patch;
    } else {
      return filter<K>() * upto_kth<K - 1>(patch);
    }
  }

private:

  std::tuple<TFilters...> m_filters;
};

/**
 * @brief Combine two filters.
 */
template <typename TOp, typename TWindow, typename UOp, typename UWindow>
FilterSeq<Filter<TOp, TWindow>, Filter<UOp, UWindow>> operator*(Filter<TOp, TWindow> lhs, Filter<UOp, UWindow> rhs)
{
  return FilterSeq<Filter<TOp, TWindow>, Filter<UOp, UWindow>>(std::make_tuple(std::move(lhs), std::move(rhs)));
}

/**
 * @brief Create a filter made of identical 1D correlation kernels along given axes.
 * 
 * Axis need not be different, e.g. to define some iterative kernel.
 */
template <typename T, Index I0, Index... Is>
auto correlation_along(std::vector<T> values)
{
  if constexpr (sizeof...(Is) == 0) {
    const auto radius = values.size() / 2;
    auto front = Position<I0 + 1>::zero();
    front[I0] = -radius; // FIXME +1?
    auto back = Position<I0 + 1>::zero();
    back[I0] = values.size() - radius - 1;
    return correlation<T, I0 + 1>(values.data(), {front, back}); // FIXME line-based window
  } else {
    return correlation_along<T, I0>(values) * correlation_along<T, Is...>(values);
  }
}

/**
 * @brief Create a filter made of identical 1D convolution kernels along given axes.
 * 
 * Axis need not be different, e.g. to define some iterative kernel.
 */
template <typename T, Index I0, Index... Is>
auto convolution_along(std::vector<T> values)
{
  if constexpr (sizeof...(Is) == 0) {
    const auto radius = values.size() / 2;
    auto front = Position<I0 + 1>::zero();
    front[I0] = -radius; // FIXME +1?
    auto back = Position<I0 + 1>::zero();
    back[I0] = values.size() - radius - 1;
    return convolution<T, I0 + 1>(values.data(), {front, back}); // FIXME line-based window
  } else {
    return convolution_along<T, I0>(values) * convolution_along<T, Is...>(values);
  }
}

/**
 * @brief Make a Prewitt filter along given axes.
 * @tparam T The filter output type
 * @tparam IDerivation The derivation axis
 * @tparam IAveraging The possibly multiple averaging axes
 * @param sign The differentiation sign (-1 or 1)
 * 
 * The convolution kernel along the `IAveraging` axes is `{1, 1, 1}` and that along `IDerivation` is `{sign, 0, -sign}`.
 * For differenciation in the increasing-index direction, keep `sign = 1`;
 * for the opposite direction, set `sign = -1`.
 * 
 * For example, to compute the derivative along axis 1 backward, while averaging along axes 0 and 2, do:
 * \code
 * auto kernel = prewitt_filter<int, 1, 0, 2>(-1);
 * auto dy = kernel * raster;
 * \endcode
 * 
 * @see `sobel_filter()`
 * @see `scharr_filter()`
 */
template <typename T, Index IDerivation, Index... IAveraging>
auto prewitt_filter(T sign = 1)
{
  const auto derivation = convolution_along<T, IDerivation>({sign, 0, -sign});
  const auto averaging = convolution_along<T, IAveraging...>({1, 1, 1});
  return derivation * averaging;
}

/**
 * @brief Make a Sobel filter along given axes.
 * 
 * The convolution kernel along the `IAveraging` axes is `{1, 2, 1}` and that along `IDerivation` is `{sign, 0, -sign}`.
 * 
 * @see `prewitt_filter()`
 * @see `scharr_filter()`
 */
template <typename T, Index IDerivation, Index... IAveraging>
auto sobel_filter(T sign = 1)
{
  const auto derivation = convolution_along<T, IDerivation>({sign, 0, -sign});
  const auto averaging = convolution_along<T, IAveraging...>({1, 2, 1});
  return derivation * averaging;
}

/**
 * @brief Make a Scharr filter along given axes.
 * 
 * The kernel along the `IAveraging` axes is `{3, 10, 3}` and that along `IDerivation` is `{sign, 0, -sign}`.
 * 
 * @see `prewitt_filter()`
 * @see `sobel_filter()`
 */
template <typename T, Index IDerivation, Index... IAveraging>
auto scharr_filter(T sign = 1)
{
  const auto derivation = convolution_along<T, IDerivation>({sign, 0, -sign});
  const auto averaging = convolution_along<T, IAveraging...>({3, 10, 3});
  return derivation * averaging;
}

/**
 * @brief Make a Laplacian filter along given axes.
 * 
 * The convolution kernel is built as a sum of 1D kernels `{sign, -2 * sign, sign}`.
 */
template <typename T, Index... Is>
auto laplacian_filter(T sign = 1)
{
  return convolution_along<T, Is...>({sign, sign * -2, sign});
  // FIXME return (convolution_along<T, Is>({sign, sign * -2, sign}) + ...);
}

} // namespace Linx

#endif
