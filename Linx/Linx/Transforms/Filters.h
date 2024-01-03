// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_FILTERLIB_H
#define _LINXTRANSFORMS_FILTERLIB_H

#include "Linx/Base/TypeUtils.h"
#include "Linx/Transforms/FilterAgg.h"
#include "Linx/Transforms/FilterSeq.h"
#include "Linx/Transforms/SimpleFilter.h"

namespace Linx {

/**
 * @ingroup filtering
 * @brief Correlation operation.
 */
template <typename T>
class Correlation {
public:

  /**
   * @brief The correlation type.
   */
  using Value = T;

  /**
   * @brief Constructor.
   */
  template <typename TRange>
  Correlation(TRange&& values) : m_kernel(std::move(values))
  {
    if constexpr (is_complex<T>()) {
      for (auto& e : m_kernel) {
        e = std::conj(e);
      }
    }
  }

  /**
   * @brief Perform the operation on given neighborhood values.
   */
  template <typename TIn>
  inline Value operator()(const TIn& neighbors) const
  {
    return std::inner_product(m_kernel.begin(), m_kernel.end(), neighbors.begin(), Value {});
  }

private:

  /**
   * @brief The kernel.
   */
  std::vector<T> m_kernel;
};

/**
 * @ingroup filtering
 * @brief Convolution operation.
 */
template <typename T>
class Convolution {
public:

  /**
   * @brief The convolution type.
   */
  using Value = T;

  /**
   * @brief Constructor.
   */
  template <typename TRange>
  Convolution(TRange&& values) : m_kernel(std::move(values))
  {}

  /**
   * @brief Perform the operation on given neighborhood values.
   */
  template <typename TIn>
  inline Value operator()(const TIn& neighbors) const
  {
    return std::inner_product(m_kernel.rbegin(), m_kernel.rend(), neighbors.begin(), Value {});
  }

private:

  /**
   * @brief The kernel.
   */
  std::vector<T> m_kernel;
};

/**
 * @ingroup filtering
 * @brief Mean filtering.
 */
template <typename T>
struct MeanFilter {
  using Value = T; // FIXME deduce from operator()

  template <typename TIn>
  T operator()(const TIn& neighbors) const
  {
    return std::accumulate(neighbors.begin(), neighbors.end(), T()) / neighbors.size();
  }
};

/**
 * @ingroup filtering
 * @brief Median filtering.
 */
template <typename T>
struct MedianFilter {
  using Value = T;

  template <typename TIn>
  T operator()(const TIn& neighbors) const
  {
    std::vector<Value> v(neighbors.begin(), neighbors.end());
    const auto size = v.size();
    auto b = v.data();
    auto e = b + size;
    auto n = b + size / 2;
    std::nth_element(b, n, e);
    if (size % 2 == 1) {
      return *n;
    }
    std::nth_element(b, n + 1, e);
    return (*n + *(n + 1)) * .5;
  }
};

/**
 * @ingroup filtering
 * @brief Erosion (i.e. min filtering).
 */
template <typename T>
struct Erosion {
  using Value = T;

  template <typename TIn>
  T operator()(const TIn& neighbors) const
  {
    return *std::min_element(neighbors.begin(), neighbors.end());
  }
};

/**
 * @ingroup filtering
 * @brief Dilation (i.e. max filtering).
 */
template <typename T>
struct Dilation {
  using Value = T;

  template <typename TIn>
  T operator()(const TIn& neighbors) const
  {
    return *std::max_element(neighbors.begin(), neighbors.end());
  }
};

/**
 * @ingroup filtering
 * @brief Make a convolution kernel from values and a window.
 */
template <typename T, Index N = 2>
auto convolution(const T* values, Box<N> window)
{
  const T* end = values + window.size();
  return SimpleFilter<Convolution<T>, Box<N>>(std::vector<T>(values, end), std::move(window));
}

/**
 * @ingroup filtering
 * @brief Make a convolution kernel from a raster and origin position.
 */
template <typename T, Index N, typename THolder>
auto convolution(const Raster<T, N, THolder>& values, Position<N> origin)
{
  return convolution(values.data(), values.domain() - origin);
}

/**
 * @ingroup filtering
 * @brief Make a convolution kernel from a raster, with centered origin.
 * 
 * In case of even lengths, origin position is rounded down.
 */
template <typename T, Index N, typename THolder>
auto convolution(const Raster<T, N, THolder>& values)
{
  return convolution(values.data(), values.domain() - (values.shape() - 1) / 2);
}

/**
 * @ingroup filtering
 * @brief Make a correlation kernel from values and a window.
 */
template <typename T, Index N = 2>
auto correlation(const T* values, Box<N> window)
{
  const T* end = values + window.size();
  return SimpleFilter<Correlation<T>, Box<N>>(std::vector<T>(values, end), std::move(window));
}

/**
 * @ingroup filtering
 * @brief Make a correlation kernel from a raster and origin position.
 */
template <typename T, Index N, typename THolder>
auto correlation(const Raster<T, N, THolder>& values, Position<N> origin)
{
  return correlation(values.data(), values.domain() - origin);
}

/**
 * @ingroup filtering
 * @brief Make a correlation kernel from a raster, with centered origin.
 * 
 * In case of even lengths, origin position is rounded down.
 */
template <typename T, Index N, typename THolder>
auto correlation(const Raster<T, N, THolder>& values)
{
  return correlation(values.data(), values.domain() - (values.shape() - 1) / 2);
}

/**
 * @ingroup filtering
 * @brief Create a filter made of identical 1D correlation kernels along given axes.
 * 
 * Axes need not be different, e.g. to define some iterative kernel.
 */
template <typename T, Index I0, Index... Is>
auto correlation_along(const std::vector<T>& values)
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
 * @ingroup filtering
 * @brief Create a filter made of identical 1D convolution kernels along given axes.
 * 
 * Axis need not be different, e.g. to define some iterative kernel.
 */
template <typename T, Index I0, Index... Is>
auto convolution_along(const std::vector<T>& values)
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
 * @ingroup filtering
 * @brief Make a Prewitt gradient filter along given axes.
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
 * auto kernel = prewitt_gradient<int, 1, 0, 2>(-1);
 * auto dy = kernel * raster;
 * \endcode
 * 
 * @see `sobel_gradient()`
 * @see `scharr_gradient()`
 */
template <typename T, Index IDerivation, Index... IAveraging>
auto prewitt_gradient(T sign = 1) // FIXME rename as prewitt_gradient
{
  const auto derivation = convolution_along<T, IDerivation>({sign, 0, -sign});
  const auto averaging = convolution_along<T, IAveraging...>({1, 1, 1});
  return derivation * averaging;
}

/**
 * @ingroup filtering
 * @brief Make a Sobel gradient filter along given axes.
 * 
 * The convolution kernel along the `IAveraging` axes is `{1, 2, 1}` and that along `IDerivation` is `{sign, 0, -sign}`.
 * 
 * @see `prewitt_gradient()`
 * @see `scharr_gradient()`
 */
template <typename T, Index IDerivation, Index... IAveraging>
auto sobel_gradient(T sign = 1) // FIXME rename as sobel_gradient
{
  const auto derivation = convolution_along<T, IDerivation>({sign, 0, -sign});
  const auto averaging = convolution_along<T, IAveraging...>({1, 2, 1});
  return derivation * averaging;
}

/**
 * @ingroup filtering
 * @brief Make a Scharr gradient filter along given axes.
 * 
 * The kernel along the `IAveraging` axes is `{3, 10, 3}` and that along `IDerivation` is `{sign, 0, -sign}`.
 * 
 * @see `prewitt_gradient()`
 * @see `sobel_gradient()`
 */
template <typename T, Index IDerivation, Index... IAveraging>
auto scharr_gradient(T sign = 1) // FIXME rename as scharr_gradient
{
  const auto derivation = convolution_along<T, IDerivation>({sign, 0, -sign});
  const auto averaging = convolution_along<T, IAveraging...>({3, 10, 3});
  return derivation * averaging;
}

/**
 * @ingroup filtering
 * @brief Make a Laplace operator along given axes.
 * 
 * The convolution kernel is built as a sum of 1D kernels `{sign, -2 * sign, sign}`.
 */
template <typename T, Index... Is>
auto laplace_operator(T sign = 1)
{
  return FilterAgg(std::plus<T>(), convolution_along<T, Is>({sign, sign * -2, sign})...);
}

/**
 * @ingroup filtering
 */
template <typename T, typename TWindow>
SimpleFilter<MeanFilter<T>, TWindow> mean_filter(TWindow window)
{
  return SimpleFilter<MeanFilter<T>, TWindow>(MeanFilter<T> {}, std::move(window)); // FIXME separable
}

/**
 * @ingroup filtering
 */
template <typename T, typename TWindow>
SimpleFilter<MedianFilter<T>, TWindow> median_filter(TWindow window)
{
  return SimpleFilter<MedianFilter<T>, TWindow>(MedianFilter<T> {}, std::move(window));
}

/**
 * @ingroup filtering
 */
template <typename T, typename TWindow>
SimpleFilter<Erosion<T>, TWindow> erosion(TWindow window)
{
  return SimpleFilter<Erosion<T>, TWindow>(Erosion<T> {}, std::move(window));
}

/**
 * @ingroup filtering
 */
template <typename T, typename TWindow>
SimpleFilter<Dilation<T>, TWindow> dilation(TWindow window)
{
  return SimpleFilter<Dilation<T>, TWindow>(Dilation<T> {}, std::move(window));
}

} // namespace Linx

#endif
