// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_KERNEL_H
#define _LITLTRANSFORMS_KERNEL_H

#include "LitlTransforms/impl/Filter.h"

namespace Litl {

/**
 * @brief A kernel which can be used for convolution or cross-correlation.
 */
template <typename T, Index N = 2>
class Kernel { // FIXME DataContainer?

public:
  /**
   * @brief The value type.
   */
  using Value = T;

  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = N;

  /**
   * @brief Constructor.
   */
  Kernel(const T* values, Box<N> window) : m_values(values, values + window.size()), m_window(std::move(window)) {}

  /**
   * @brief Get the window.
   */
  const Box<Dimension>& window() const {
    return m_window;
  }

  /**
   * @brief View the kernel values as a `Raster`.
   */
  PtrRaster<const Value, Dimension> raster() const {
    return PtrRaster<const Value, Dimension>(m_window.shape(), m_values.data());
  }

  /**
   * @copybrief raster()
   */
  PtrRaster<Value, Dimension> raster() {
    return PtrRaster<Value, Dimension>(m_window.shape(), m_values.data());
  }

  /**
   * @brief Convolve an input raster.
   */
  template <typename TIn>
  auto operator*(const TIn& in) const {
    return convolve() * in;
  }

  /**
   * @brief Make a convolution filter.
   */
  auto convolve() const {
    return filterize(*this, [&](const auto& neighbors) -> T {
      return std::inner_product(m_values.rbegin(), m_values.rend(), neighbors.begin(), T {});
    });
  }

  /**
   * @brief Make a correlation filter.
   */
  auto correlate() const {
    return filterize(*this, [&](const auto& neighbors) -> T {
      return std::inner_product(m_values.begin(), m_values.end(), neighbors.begin(), T {});
    });
  }

private:
  /**
   * @brief The correlation values.
   */
  std::vector<T> m_values;

  /**
   * @brief The correlation window.
   */
  Box<N> m_window;
};

/**
 * @relates Kernel
 * @brief Make a kernel from values and a window.
 */
template <typename T, Index N = 2>
Kernel<T, N> kernelize(const T* values, Box<N> window) {
  return Kernel<T, N>(values, std::move(window));
}

/**
 * @relates Kernel
 * @brief Make a kernel from a raster and origin position.
 */
template <typename T, Index N, typename THolder>
Kernel<T, N> kernelize(const Raster<T, N, THolder>& values, Position<N> origin) {
  return kernelize(values.data(), values.domain() - origin);
}

/**
 * @relates Kernel
 * @brief Make a kernel from a raster, with centered origin.
 * 
 * In case of even lengths, origin position is rounded down.
 */
template <typename T, Index N, typename THolder>
Kernel<T, N> kernelize(const Raster<T, N, THolder>& values) {
  return kernelize(values.data(), values.domain() - (values.shape() - 1) / 2);
}

} // namespace Litl

#endif
