// Copyright (C) 2022, Antoine Basset
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERFILTER_KERNEL_H
#define _RASTERFILTER_KERNEL_H

#include "LitlRaster/Raster.h"

namespace Litl {

struct OutOfBoundsCrop {};

template <typename T>
struct OutOfBoundsConstant { // FIXME to Extrapolation.h
  T value {0};
};

/**
 * @brief A kernel which can be used for convolution or auto-correlation.
 * @tparam T The value type
 * @tparam N The dimension
 * @tparam TBoundary The boundary conditions
 */
template <typename T, Index N, typename TBoundary = OutOfBoundsConstant<T>>
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
  Kernel(TRaster&& values, const Position<N>& origin, TBoundary boundaryConditions = TBoundary()) :
      m_values(values), m_origin(origin), m_boundary(boundaryConditions) {}

  /**
   * @brief Auto-correlate a raster with the kernel.
   */
  template <typename TIn>
  Raster<Value, Dimension> correlate(const TIn& in) {
    Raster<Value, Dimension> out(in.shape());
    correlateTo(in, out);
    return out;
  }

  /**
   * @copybrief correlate()
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
    correlateTo(in, res);
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
   * @brief Sparsely correlate an input raster by a given 1D kernel.
   */
  template <typename TIn, typename TKernel, typename TOut>
  void sparseCorrelateWith1dTo(const TIn& in, const TKernel& kernel, TOut& out) {
    auto inUnit = in;
    inUnit.step(1); // For inner_product
    auto inIt = inUnit.begin();
    auto inMinIt = inIt;
    inMinIt -= in.front();
    inIt -= kernel.backward;
    auto outIt = out.begin();
    auto i = in.front();
    for (; i < kernel.backward; i += in.step(), inIt += in.step(), ++outIt) {
      *outIt = std::inner_product(kernel.center - i, kernel.end(), inMinIt, kernel.bias);
    }
    for (; i <= in.size() - kernel.forward - in.step(); i += in.step(), inIt += in.step(), ++outIt) {
      *outIt = std::inner_product(kernel.begin(), kernel.end(), inIt, kernel.bias);
    }
    for (; i <= in.to(); i += in.step(), inIt += in.step(), ++outIt) {
      *outIt = std::inner_product(kernel.begin(), kernel.center + (in.size() - i), inIt, kernel.bias);
    }
  }

  /**
   * @brief The correlation values.
   */
  Raster<T, N> m_values;

  /**
   * @brief The origin coordinates.
   */
  Position<N> m_origin;

  /**
   * @brief The boundary conditions.
   */
  TBoundary m_boundary;
};

} // namespace Litl

#endif
