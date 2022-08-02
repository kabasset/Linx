// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_KERNEL_H
#define _LITLTRANSFORMS_KERNEL_H

#include "LitlRaster/Raster.h"
#include "LitlTransforms/Interpolation.h"

namespace Litl {

/**
 * @brief A kernel which can be used for convolution or cross-correlation.
 */
template <typename T, Index N = 2>
class Kernel { // FIXME DataContainer

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
   * @brief Get the shape.
   */
  Position<Dimension> shape() const {
    return m_window.shape();
  }

  /**
   * @brief View the kernel values as a `Raster`.
   */
  const PtrRaster<const Value, Dimension> raster() const {
    return PtrRaster<const Value, Dimension>(m_window.shape(), m_values.data());
  }

  /**
   * @copybrief raster()
   */
  PtrRaster<Value, Dimension> raster() {
    return PtrRaster<Value, Dimension>(m_window.shape(), m_values.data());
  }

  /**
   * @brief Cross-correlate a raster with the kernel.
   * @note
   * Correlation is like convolution with the reversed kernel.
   */
  template <typename TRaster, typename TMethod>
  Raster<Value, Dimension> operator*(const Extrapolator<TRaster, TMethod>& in) {
    Raster<Value, Dimension> out(in.shape());
    correlateTo(in, out);
    return out;
  }

  /**
   * @brief Cross-correlate a raster with the kernel into an output raster.
   */
  template <typename TRaster, typename TMethod, typename TOut>
  void correlateTo(const Extrapolator<TRaster, TMethod>& in, TOut& out) {
    const auto inner = in.domain() - m_window;
    const auto outers = inner.surround(m_window);
    correlateRegionTo(Litl::rasterize(in), inner, out);
    for (const auto& o : outers) {
      correlateRegionTo(in, o, out);
    }
  }

  /**
   * @brief Correlate an input raster or extrapolator over a given box.
   */
  template <typename TIn, typename TOut>
  void correlateRegionTo(const TIn& in, const Box<N>& region, TOut& out) {
    if (region.size() < 0) {
      return;
    }
    auto patch = in.subraster(m_window);
    for (const auto& p : region) {
      patch.shift(p);
      out[p] = std::inner_product(m_values.begin(), m_values.end(), patch.begin(), T {});
      // FIXME replace out[p] with an iterator
      patch.shiftBack(p);
    }
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
 * @details
 * In case of even lengths, origin position is rounded down.
 */
template <typename T, Index N, typename THolder>
Kernel<T, N> kernelize(const Raster<T, N, THolder>& values) {
  return kernelize(values.data(), values.domain() - values.shape() / 2);
}

} // namespace Litl

#endif
