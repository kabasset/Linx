// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERFILTER_CONVOLUTION_H
#define _RASTERFILTER_CONVOLUTION_H

#include "Raster/Raster.h"
#include "Raster/Sampling.h"

#include <algorithm> // reverse

namespace Cnes {

struct OutOfBoundsCrop {};

template <typename T, typename TOob = OutOfBoundsCrop>
class Kernel1d : public DataContainer<T, StdHolder<std::vector<T>>, Kernel1d<T, TOob>> {

public:
  /**
   * @brief Constructor.
   */
  Kernel1d(std::vector<T> values, Index origin, TOob oob = TOob()) :
      DataContainer<T, StdHolder<std::vector<T>>, Kernel1d<T, TOob>>(values), m_backward(origin),
      m_forward(this->size() - 1 - m_backward), m_bias(), m_oob(oob) {}

  /**
   * @brief Get the number of backward values.
   */
  Index backward() const {
    return m_backward;
  }

  /**
   * @brief Get the number of forward values.
   */
  Index forward() const {
    return m_forward;
  }

  /**
   * @brief Get a pointer to the data at origin.
   */
  const T* originData() const {
    return this->data() + m_backward;
  }

  /**
   * @brief Correlate a given image with the kernel along a given axis.
   */
  template <Index Axis, typename TOut = T, typename TRasterIn>
  Raster<TOut, TRasterIn::Dim> correlateAlong(const TRasterIn& in) const {
    const auto shape = in.shape();
    const auto length = shape[Axis];
    const auto stride = shapeStride<Axis>(shape);
    Raster<TOut, TRasterIn::Dim> out(shape);
    auto domain = in.domain();
    domain.back[Axis] = 0;
    for (const auto& p : domain) {
      DataSamples<const typename TRasterIn::Value> inSamples {&in[p], length, {}, stride};
      DataSamples<TOut> outSamples {&out[p], length, {}, stride};
      sparseCorrelate1dTo(inSamples, outSamples);
    }
    return out;
  }

  /**
   * @brief Sparsely correlate a given sampled image with the kernel.
   */
  template <typename TIn, typename TOut>
  void sparseCorrelate1dTo(const DataSamples<TIn>& in, DataSamples<TOut>& out)
      const { // FIXME only valid for CropOutOfBounds

    // Set iterators
    const auto step = in.step();
    DataSamples<const TIn>
        unitIn {in.data(), in.size(), {in.front(), in.back()}, in.stride()}; // step = 1 for inner_product
    auto inIt = unitIn.begin();
    auto inMinIt = inIt;
    inMinIt -= in.front();
    inIt -= m_backward;
    auto outIt = out.begin();
    auto i = in.front();

    printf("in = [%li:%li:%li:%li]\n", in.front(), in.back(), step, in.stride());

    // Backward-croped
    for (; i < m_backward; i += step, inIt += step, ++outIt) {
      *outIt = std::inner_product(originData() - i, this->end(), inMinIt, m_bias);
      printf("Begin (<%li): out[%li] = %i\n", m_backward, i, *outIt); // FIXME rm
    }

    // Central
    for (; i <= in.size() - m_forward - step; i += step, inIt += step, ++outIt) {
      *outIt = std::inner_product(this->begin(), this->end(), inIt, m_bias);
      printf("Center (<=%li): out[%li] = %i\n", in.size() - m_forward - step, i, *outIt); // FIXME rm
    }

    // Forward-croped
    for (; i <= in.back(); i += step, inIt += step, ++outIt) {
      *outIt = std::inner_product(this->begin(), originData() + (in.size() - i), inIt, m_bias);
      printf("End (<=%li): out[%li] = %i\n", in.back(), i, *outIt); // FIXME rm
    }
  }

  /**
   * @brief Correlate a given image with the kernel along the first and second axes.
   */
  template <typename TRasterIn, typename TRasterOut>
  void correlate2dTo(const TRasterIn& in, TRasterOut& out) {
    PositionSampling<TRasterIn::Dim> sampling {in.domain()};
    sparseCorrelate2dTo(in, sampling, out);
  }

  /**
   * @brief Sparsely correlate a given sampled image with the kernel along the first and second axes.
   */
  template <typename TRasterIn, typename TRasterOut>
  void
  sparseCorrelate2dTo(const TRasterIn& in, const PositionSampling<TRasterIn::Dim>& sampling, TRasterOut& out) const {

    // Set sampling
    const auto xFrom = sampling.template along<0>().front;
    const auto yFrom = std::max(0L, sampling.template along<1>().front - m_backward);
    const auto xTo = sampling.template along<0>().back;
    const auto yTo = std::min(in.template length<1>() - 1, sampling.template along<1>().back + m_forward);
    const auto xStep = sampling.template along<0>().step;
    const auto yStep = sampling.template along<1>().step;
    printf("%li-%li:%li - %li-%li:%li\n", xFrom, xTo, xStep, yFrom, yTo, yStep);

    // Convolve along x-axis
    const Index xConvolvedWidth = (xTo - xFrom + xStep) / xStep;
    const Index xConvolvedHeight = yTo - yFrom + 1;
    Raster<typename TRasterOut::Value, TRasterOut::Dim> xConvolved({xConvolvedWidth, xConvolvedHeight});
    DataSamples<const typename TRasterIn::Value> inSamples(
        &in[{0, yFrom}],
        in.template length<0>(),
        {xFrom, xTo, xStep});
    printf("inSamples: %li-%li:%li:%li\n", inSamples.front(), inSamples.back(), inSamples.step(), inSamples.stride());
    DataSamples<typename TRasterOut::Value> xConvolvedSamples(xConvolved.data(), xConvolvedWidth, {0, -1});
    printf(
        "xConvolvedSamples: %li-%li:%li:%li\n",
        xConvolvedSamples.front(),
        xConvolvedSamples.back(),
        xConvolvedSamples.step(),
        xConvolvedSamples.stride());
    for (Index y = yFrom; y <= yTo; ++y) {
      printf(
          "in: %li, xConvolved: %li\n",
          std::distance(in.data(), inSamples.data()),
          std::distance(xConvolved.data(), xConvolvedSamples.data()));
      sparseCorrelate1dTo(inSamples, xConvolvedSamples);
      inSamples += in.template length<0>();
      xConvolvedSamples += xConvolvedWidth;
    }

    printf("xConvolved:\n%lix%li\n", xConvolvedWidth, xConvolvedHeight);
    for (Index y = 0; y < xConvolvedHeight; ++y) {
      for (Index x = 0; x < xConvolvedWidth; ++x) {
        printf("%i ", xConvolved[{x, y}]);
      }
      printf("\n");
    }

    // Convolve along y-axis
    DataSamples<typename TRasterOut::Value> ySamples(
        xConvolved.data(),
        xConvolvedHeight,
        {sampling.template along<1>().front - yFrom, sampling.template along<1>().back - yFrom, yStep},
        xConvolvedWidth);
    printf("ySamples: %li-%li:%li:%li\n", ySamples.front(), ySamples.back(), ySamples.step(), ySamples.stride());
    DataSamples<typename TRasterOut::Value>
        outSamples(out.data(), out.template length<1>(), {0, -1}, out.template length<0>());
    printf(
        "outSamples: %li-%li:%li:%li\n",
        outSamples.front(),
        outSamples.back(),
        outSamples.step(),
        outSamples.stride());
    for (Index x = xFrom; x <= xTo; x += xStep) {
      printf(
          "xConvolved: %li, out: %li\n",
          std::distance(xConvolved.data(), ySamples.data()),
          std::distance(out.data(), outSamples.data()));
      sparseCorrelate1dTo(ySamples, outSamples);
      ySamples += 1;
      outSamples += 1;
    }

    printf("out:\n%lix%li\n", out.template length<0>(), out.template length<1>());
    for (Index y = 0; y < out.template length<1>(); ++y) {
      for (Index x = 0; x < out.template length<0>(); ++x) {
        printf("%i ", out[{x, y}]);
      }
      printf("\n");
    }
  }

private:
  Index m_backward;
  Index m_forward;
  T m_bias;
  TOob m_oob;
};

} // namespace Cnes

#endif // _RASTERFILTER_CONVOLUTION_H
