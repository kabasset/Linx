// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_KERNEL_H
#define _LITLTRANSFORMS_KERNEL_H

#include "LitlRaster/Raster.h"

namespace Litl {

/**
 * @brief A kernel which can be used for convolution or auto-correlation.
 * @tparam T The value type
 * @tparam N The dimension
 * @tparam TBoundary The boundary conditions
 */
template <typename T, Index N = 2>
class Kernel {

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
  template <typename TRaster>
  Kernel(TRaster&& values, const Position<N>& origin) : m_values(values), m_origin(origin) {}

  /**
   * @brief Auto-correlate a raster with the kernel.
   */
  template <typename TIn, typename THolder>
  Raster<Value, Dimension> operator*(const Raster<TIn, N, THolder>& in) {
    Raster<Value, Dimension> out(in.shape());
    correlateTo(in, out);
    return out;
  }

  /**
   * @copybrief operator*()
   */
  template <typename TIn, typename TOut>
  void correlateTo(const TIn& in, TOut& out) {
    correlateWithTo(in, m_values, out);
  }

  /**
   * @brief Sparsely auto-correlate a raster with the kernel.
   */
  template <typename TIn, typename TOut>
  void sparseCorrelateTo(const TIn& in, TOut& out) {}

  /**
   * @brief Convolve a raster with the kernel.
   */
  template <typename TIn>
  Raster<Value, Dimension> convolve(const TIn& in) {
    Raster<Value, Dimension> res(in.shape());
    convolveTo(in, res);
    return res;
  }

  /**
   * @copybrief convolve()
   */
  template <typename TIn, typename TOut>
  void convolveTo(const TIn& in, TOut& out) {
    auto correlation = m_values;
    std::reverse(correlation.begin(), correlation.end());
    correlateWithTo(in, correlation, out);
  }

  /**
   * @brief Sparsely convolve a raster with the kernel.
   */
  template <typename TIn, typename TOut>
  void sparseConvolveTo(const TIn& in, TOut& out) {
    auto correlation = m_values;
    std::reverse(correlation.begin(), correlation.end());
    sparseCorrelateWithTo(in, correlation, out);
  }

private:
  /**
   * @brief Correlate an input raster with a given kernel.
   */
  template <typename TRasterIn, typename TKernel, typename TRasterOut>
  void correlateWithTo(const TRasterIn& in, const TKernel& kernel, TRasterOut& out) {
    // FIXME
  }

  /**
   * @brief The correlation values.
   */
  Raster<T, N> m_values;

  /**
   * @brief The origin coordinates.
   */
  Position<N> m_origin;
};

} // namespace Litl

#endif
