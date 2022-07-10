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
class RasterKernel { // FIXME DataContainer or ShapedDataContaier

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
  RasterKernel(const T* values, Box<N> window) :
      m_values(values, values + window.size()), m_window(std::move(window)) {}

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
   * @copybrief operator*()
   */
  template <typename TRaster, typename TMethod, typename TOut>
  void correlateTo(const Extrapolator<TRaster, TMethod>& in, TOut& out) {
    const auto inner = in.domain() - m_window;
    const auto outers = inner.surround(m_window);
    correlateWithoutExtrapolation(raster(in), std::move(inner), out);
    for (const auto& o : outers) {
      correlateWithExtrapolation(in, o, out);
    }
  }

private:
  /**
   * @brief Correlate an input raster over a given region.
   */
  template <typename TRasterIn, typename TRasterOut>
  void correlateWithoutExtrapolation(const TRasterIn& in, Box<N> box, TRasterOut& out) {

    if (box.size() == 0) {
      return;
    }

    // Compute constants
    auto boxWidth = box.template length<0>();
    box.project(); // Keep front hyperplane only
    const auto inWidth = in.template length<0>();
    const auto kWidth = m_window.template length<0>();
    const auto kRowCount = m_window.size() / kWidth;

    // Prepare iterators
    auto inIt = in.data();
    auto outIt = &out[box.front()];
    auto kIt = m_values.data();
    const auto distance = outIt - inIt;

    // Loop over the hyperplane
    for (const auto& p : box) {

      // Loop over the rows which begin in the hyperplane
      outIt = &out[p];
      for (Index i = boxWidth; i > 0; --i, ++outIt, inIt = outIt - distance, kIt = m_values.data()) {

        // Compute the weighted sum row by row
        T sum {};
        for (Index j = kRowCount; j > 0; --j, inIt += inWidth, kIt += kWidth) {
          sum = std::inner_product(kIt, kIt + kWidth, inIt, sum);
        }
        *outIt = sum;
      }
    }
  }

  /**
   * @brief Correlate an input raster over a given region.
   */
  template <typename TExtrapolatorIn, typename TRasterOut>
  void correlateWithExtrapolation(const TExtrapolatorIn& in, const Box<N>& box, TRasterOut& out) {

    if (box.size() == 0) {
      return;
    }

    // Allocate extrapolation buffer
    std::vector<T> buffer;
    buffer.reserve(m_values.size());

    // Prepare iterators
    const auto bBegin = buffer.begin();
    const auto kBegin = m_values.begin();
    const auto kEnd = m_values.end();

    // Loop over the box
    for (const auto& p : box) {

      // Fill the buffer
      buffer.clear(); // Memory is not affected
      for (const auto& q : m_window + p) {
        buffer.push_back(in[q]);
      }

      // Compute the weighted sum
      out[p] = std::inner_product(kBegin, kEnd, bBegin, T {});
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
 * @brief Make a kernel.
 */
template <typename T, Index N = 2>
RasterKernel<T, N> kernelize(const T* values, Box<N> window) {
  return RasterKernel<T, N>(values, std::move(window));
}

/**
 * @brief Make a kernel.
 */
template <typename T, Index N, typename THolder>
RasterKernel<T, N> kernelize(const Raster<T, N, THolder>& values, Position<N> origin) {
  return RasterKernel<T, N>(values.data(), values.domain() - origin);
}

/**
 * @brief Make a kernel.
 */
template <typename T, Index N, typename THolder>
RasterKernel<T, N> kernelize(const Raster<T, N, THolder>& values) {
  return RasterKernel<T, N>(values.data(), values.domain() - values.shape() / 2);
}

} // namespace Litl

#endif
