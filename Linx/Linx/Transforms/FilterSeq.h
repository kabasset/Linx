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
class FilterSeq {
  template <typename... UFilters>
  friend class FilterSeq; // FIXME useful?

public:

  /**
   * @brief The last filter value type.
   */
  using Value = typename std::decay_t<decltype((std::declval<TFilters>(), ...))>::Value;

  /**
   * @brief The logical dimension of the combined kernel.
   */
  static constexpr Index Dimension = std::max({TFilters::Dimension...});

  /**
   * @brief Constructor.
   */
  template <typename... TArgs>
  explicit FilterSeq(TArgs&&... args) : m_filters(std::forward<Ts>(args)...)
  {}

  /// @group_properties

  /**
   * @brief The logical window of the kernel.
   */
  Box<Dimension> window() const
  {
    auto front = Position<Dimension>::zero();
    auto back = Position<Dimension>::zero();
    seq_foreach(m_filters, [&](const auto& k) {
      front[k.Axis] = std::min(front[k.Axis], k.window().front());
      back[k.Axis] = std::max(back[k.Axis], k.window().back());
    });
    return {front, back};
  }

  /**
   * @brief Convolve the separable components as a single ND kernel.
   */
  auto compose() const
  { // FIXME inherit KernelOp
    const auto w = window();
    const auto o = -w.front();
    auto raster = Raster<Value, Dimension>(w.shape());
    raster[o] = T(1); // FIXME or back-o?
    auto impulse = *this * extrapolation(raster, 0);
    return convolution(impulse.reverse(), o); // FIXME use KernelOp
  }

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
  operator*(Filter<T, TWindow>&& lhs, const FilterSeq<T, I0, Is...>& rhs)
  {
    return FilterSeq<Filter<T, TWindow>, TFilters...>(
        std::tuple_cat(std::make_tuple(std::forward<Filter<T, TWindow>>(lhs)), rhs.m_filters));
  }

  /**
   * @brief Apply the correlation kernels to an input extrapolator.
   */
  template <typename TIn>
  Raster<std::decay_t<typename TIn::Value>, TIn::Dimension> operator*(const TIn& in) const
  {
    Raster<std::decay_t<typename TIn::Value>, TIn::Dimension> out(in.shape());
    correlate_to(in, out);
    return out;
  }

  /**
   * @copydoc correlate()
   */
  template <typename TIn, typename TOut>
  void correlate_to(const TIn& in, TOut& out) const
  {
    correlate_kernel_seq<TIn, TOut, I0, Is...>(in, out);
  }

private:

  template <typename TIn, typename TOut, Index J0, Index... Js>
  void correlate_kernel_seq(const TIn& in, TOut& out) const
  {
    // FIXME extrapolate once for all and correlate_crop
    const auto tmp = correlate_kth_kernel<TIn, TOut, sizeof...(Is) - sizeof...(Js)>(in);
    const auto& method = in.method();
    const Extrapolation<TOut, decltype(method)> extrapolator(tmp, method);
    correlate_kernel_seq<decltype(extrapolator), TOut, Js...>(extrapolator, out);
  }

  template <typename TIn, typename TOut>
  void correlate_kernel_seq(const TIn& in, TOut& out) const
  {
    out = rasterize(in); // FIXME swap? move?
  }

  template <typename TIn, typename TOut, std::size_t K>
  TOut correlate_kth_kernel(const TIn& in) const
  {
    return std::get<K>(m_filters) * in;
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
  return FilterSeq<Filter<TOp, TWindow>, Filter<UOp, UWindow>>(std::make_tuple(std::moce(lhs), std::move(rhs)));
}

template <typename T, Index I>
Filter<Correlation<T>, Box<I + 1>> correlation_along(TIn values)
{
  const auto radius = values.size() / 2;
  auto front = Position<I + 1>::zero();
  front[I] = -radius; // FIXME +1?
  auto back = Position<I + 1>::zero();
  back[I] = values.size() - radius - 1;
  return correlation<T>(values, {front, back}); // FIXME line-based window
}

template <typename T, Index I0, Index... Is>
auto correlation_along(TIn values)
{
  return correlation_along<T, I0>(values) * correlation_along<T, Is...>(values);
}

/**
 * @brief Make a Prewitt filter along given axes.
 * 
 * The kernel along the `IAveraging` axes is `{1, 1, 1}` and that along `IDerivation` is `{-sign, 0, sign}`.
 * @see `sobel()`
 */
template <Index IDerivation, Index... IAveraging>
static FilterSeq prewitt_filter(T sign = 1)
{
  const auto derivation = correlation_along<T, IDerivation>({-sign, 0, sign});
  const auto averaging = correlation_along<T, IAveraging...>({1, 1, 1});
  return derivation * averaging;
}

/**
 * @brief Make a Sobel filter along given axes.
 * @param sign The differentiation sign (-1 or 1)
 * 
 * The kernel along the `IAveraging` axes is `{1, 2, 1}` and that along `IDerivation` is `{-sign, 0, sign}`.
 * For differenciation in the increasing-index direction, keep `sign = 1`;
 * for the opposite direction, set `sign = -1`.
 * 
 * For example, to compute the derivative along axis 1 backward, while averaging along axes 0 and 2, do:
 * \code
 * auto kernel = sobel_filter<int, 1, 0, 2>(-1);
 * auto dy = kernel * raster;
 * \endcode
 */
template <Index IDerivation, Index... IAveraging>
static auto sobel_filter(T sign = 1)
{
  const auto derivation = correlation_along<T, IDerivation>({-sign, 0, sign});
  const auto averaging = correlation_along<T, IAveraging...>({1, 2, 1});
  return derivation * averaging;
}

/**
 * @brief Make a Scharr filter along given axes.
 * 
 * The kernel along the `IAveraging` axes is `{3, 10, 3}` and that along `IDerivation` is `{-sign, 0, sign}`.
 * @see `sobel()`
 */
template <Index IDerivation, Index... IAveraging>
static auto scharr_filter(T sign = 1)
{
  const auto derivation = OrientedKernel<T, IDerivation>({-sign, 0, sign});
  const auto averaging = FilterSeq<T, Is...>({3, 10, 3});
  return derivation * averaging;
}

/**
 * @brief Make a separable Laplacian filter along given axes.
 * 
 * The kernel is built as a sequence of 1D kernels `{1, -2, 1}` if `sign` is 1,
 * or `{-1, 2, -1}` if sign is -1.
 */
template <Index... Is>
static auto separable_laplacian_filter(T sign = 1)
{
  return FilterSeq<T, I0, Is...>({sign, sign * -2, sign});
}

} // namespace Linx

#endif
