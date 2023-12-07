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
   * @brief The logical dimension of the composed kernel.
   */
  static constexpr Index Dimension = std::max({TFilters::Dimension...});

  /**
   * @brief Constructor.
   * 
   * @note Filter sequences are better constructed by multiplying simple filters.
   */
  template <typename... TArgs>
  explicit FilterSeq(TArgs&&... args) : m_filters(std::forward<TArgs>(args)...)
  {}

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
      const auto& w = k.window();
      for (Index i = 0; i < w.dimension(); ++i) {
        front[i] = std::min(front[i], w.front()[i]);
        back[i] = std::max(back[i], w.back()[i]);
      }
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
    raster[o] = Value(1); // FIXME or back-o?
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
  operator*(Filter<T, TWindow>&& lhs, const FilterSeq<TFilters...>& rhs)
  {
    return FilterSeq<Filter<T, TWindow>, TFilters...>(
        std::tuple_cat(std::make_tuple(std::forward<Filter<T, TWindow>>(lhs)), rhs.m_filters));
  }

  /**
   * @brief Apply the filters to an input extrapolator.
   */
  template <typename TIn>
  Raster<Value, TIn::Dimension> operator*(const TIn& in) const
  {
    Raster<Value, TIn::Dimension> out(in.shape());
    full_to(in, out);
    // FIXME other cases
    return out;
  }

private:

  template <typename TIn, typename TOut>
  void full_to(const TIn& in, TOut& out) const
  {
    const auto extrapolated = in.copy(Linx::box(in.domain()) + window()); // FIXME split
    crop_to(extrapolated, out);
  }

  template <typename TIn, typename TOut>
  void crop_to(const TIn& in, TOut& out) const
  {
    upto_kth_to<sizeof...(TFilters) - 1>(in, out);
  }

  template <std::size_t K, typename TIn, typename TOut>
  void upto_kth_to(const TIn& in, TOut& out) const
  {
    if constexpr (K == 0) {
      out = std::get<0>(m_filters) * in;
    } else {
      out = std::get<K>(m_filters) * upto_kth<K - 1>(in);
    }
  }

  template <std::size_t K, typename TIn>
  auto upto_kth(const TIn& in) const
  {
    if constexpr (K == 0) {
      return std::get<0>(m_filters) * in;
    } else {
      return std::get<K>(m_filters) * upto_kth<K - 1>(in);
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
 * @brief Make a Prewitt filter along given axes.
 * 
 * The kernel along the `IAveraging` axes is `{1, 1, 1}` and that along `IDerivation` is `{-sign, 0, sign}`.
 * @see `sobel()`
 */
template <typename T, Index IDerivation, Index... IAveraging>
auto prewitt_filter(T sign = 1)
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
template <typename T, Index IDerivation, Index... IAveraging>
auto sobel_filter(T sign = 1)
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
template <typename T, Index IDerivation, Index... IAveraging>
auto scharr_filter(T sign = 1)
{
  const auto derivation = correlation_along<T, IDerivation>({-sign, 0, sign});
  const auto averaging = correlation_along<T, IAveraging...>({3, 10, 3});
  return derivation * averaging;
}

/**
 * @brief Make a separable Laplacian filter along given axes.
 * 
 * The kernel is built as a sequence of 1D kernels `{1, -2, 1}` if `sign` is 1,
 * or `{-1, 2, -1}` if sign is -1.
 */
template <typename T, Index... Is>
auto separable_laplacian_filter(T sign = 1)
{
  return correlation_along<T, Is...>({sign, sign * -2, sign});
}

} // namespace Linx

#endif
