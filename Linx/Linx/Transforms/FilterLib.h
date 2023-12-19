// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_FILTERLIB_H
#define _LINXTRANSFORMS_FILTERLIB_H

#include "Linx/Base/TypeUtils.h"
#include "Linx/Transforms/Filter.h"
#include "Linx/Transforms/FilterAgg.h"
#include "Linx/Transforms/FilterSeq.h"

namespace Linx {

/**
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
 * @relatesalso Filter
 * @brief Make a convolution kernel from values and a window.
 */
template <typename T, Index N = 2>
auto convolution(const T* values, Box<N> window)
{
  const T* end = values + window.size();
  return Filter<Convolution<T>, Box<N>>(std::vector<T>(values, end), std::move(window));
}

/**
 * @relatesalso Filter
 * @brief Make a convolution kernel from a raster and origin position.
 */
template <typename T, Index N, typename THolder>
auto convolution(const Raster<T, N, THolder>& values, Position<N> origin)
{
  return convolution(values.data(), values.domain() - origin);
}

/**
 * @relatesalso Filter
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
 * @relatesalso Filter
 * @brief Make a correlation kernel from values and a window.
 */
template <typename T, Index N = 2>
auto correlation(const T* values, Box<N> window)
{
  const T* end = values + window.size();
  return Filter<Correlation<T>, Box<N>>(std::vector<T>(values, end), std::move(window));
}

/**
 * @relatesalso Filter
 * @brief Make a correlation kernel from a raster and origin position.
 */
template <typename T, Index N, typename THolder>
auto correlation(const Raster<T, N, THolder>& values, Position<N> origin)
{
  return correlation(values.data(), values.domain() - origin);
}

/**
 * @relatesalso Filter
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
 * @brief Make a Laplace operator along given axes.
 * 
 * The convolution kernel is built as a sum of 1D kernels `{sign, -2 * sign, sign}`.
 */
template <typename T, Index... Is>
auto laplace_operator(T sign = 1)
{
  return FilterAgg(std::plus<T>(), convolution_along<T, Is>({sign, sign * -2, sign})...);
}

} // namespace Linx

#endif
