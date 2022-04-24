// Copyright (C) 2022, Antoine Basset
// This file is part of Kast.Raster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _KRASTERFILTER_KERNEL1D_H
#define _KRASTERFILTER_KERNEL1D_H

#include "KRaster/Raster.h"
#include "KRaster/Sampling.h"
// #include "KRasterGeometry/Extrapolation.h"
struct CropExtrapolation {}; // FIXME rm

#include <map>
#include <vector>

namespace Kast {

/// @cond
template <typename TKernel, Index... Axes>
class Kernel1dSeq;
/// @endcond

/**
 * @brief 1D kernel for nD correlations.
 */
template <typename T, typename TExtrapolation = CropExtrapolation>
class Kernel1d : public DataContainer<T, DataContainerHolder<T, std::vector<T>>, Kernel1d<T, TExtrapolation>> {

public:
  /**
   * @brief Constructor.
   * @param values The kernel values
   * @param origin The index of the kernel origin
   * @param extrapolation The extrapolation policy
   */
  explicit Kernel1d(const std::vector<T>& values, Index origin, TExtrapolation extrapolation = TExtrapolation()) :
      DataContainer<T, DataContainerHolder<T, std::vector<T>>, Kernel1d<T, TExtrapolation>>(
          values.begin(),
          values.end()),
      m_backward(origin), m_forward(this->size() - 1 - m_backward), m_bias(), m_extrapolation(extrapolation) {}

  /**
   * @brief Constructor.
   */
  explicit Kernel1d(const std::vector<T>& values, TExtrapolation extrapolation = TExtrapolation()) :
      Kernel1d(values, values.size() / 2, extrapolation) {}

  /**
   * @brief Get the number of backward values.
   */
  Index backwardCount() const {
    return m_backward;
  }

  /**
   * @brief Get the number of forward values.
   */
  Index forwardCount() const {
    return m_forward;
  }

  /**
   * @brief Get a pointer to the data at origin.
   */
  const T* originData() const {
    return this->data() + m_backward;
  }

  /**
   * @brief Orient the kernel along given axes.
   * @details
   * For example, to apply a correlation kernel along axes 1 and 2 of a given raster `in`, do:
   * \code
   * auto out = kernel.along<1, 2>().correlate(in);
   * \endcode
   */
  template <Index... Axes>
  Kernel1dSeq<Kernel1d<T, TExtrapolation>, Axes...> along() const {
    return Kernel1dSeq<Kernel1d<T, TExtrapolation>, Axes...>(*this);
  }

  /**
   * @brief Correlate a given sampled data with the kernel.
   */
  template <typename TIn, typename TOut>
  void correlate(const DataSamples<TIn>& in, DataSamples<TOut>& out)
      const { // FIXME only valid for kernel croping extrapolation

    // Set up iterators
    const auto step = in.step();
    DataSamples<const TIn>
        unitIn {in.data(), in.size(), {in.front(), in.back()}, in.stride()}; // step = 1 for inner_product
    auto inIt = unitIn.begin();
    auto inMinIt = inIt;
    inMinIt -= in.front();
    inIt -= m_backward;
    auto outIt = out.begin();
    long i = in.front();

    // Backward-croped
    for (; i < m_backward; i += step, inIt += step, ++outIt) {
      *outIt = std::inner_product(originData() - i, this->end(), inMinIt, m_bias);
    }

    // Central
    for (; i <= in.size() - m_forward - step; i += step, inIt += step, ++outIt) {
      *outIt = std::inner_product(this->begin(), this->end(), inIt, m_bias);
    }

    // Forward-croped
    for (; i <= in.back(); i += step, inIt += step, ++outIt) {
      *outIt = std::inner_product(this->begin(), originData() + (in.size() - i), inIt, m_bias);
    }
  }

private:
  Index m_backward;
  Index m_forward;
  T m_bias;
  TExtrapolation m_extrapolation;
};

/**
 * @brief Sequence of oriented 1D kernels.
 */
template <typename TKernel, Index... Axes>
class Kernel1dSeq {
public:
  explicit Kernel1dSeq(const TKernel& kernel) : m_kernels {{Axes, &kernel}...} {}

  const TKernel& operator[](Index axis) const {
    return m_kernels.at(axis);
  }

  template <typename TOut, typename TIn, Index N, typename TContainer>
  VecRaster<TOut, N> correlate(const Raster<TIn, N, TContainer>& in) const {
    VecRaster<TOut, N> out(in.shape());
    correlateTo(in, out);
    return out;
  }

  template <typename TRasterIn, typename TRasterOut>
  void correlateTo(const TRasterIn& in, TRasterOut& out) const {
    correlateAlongSeqTo<TRasterIn, TRasterOut, Axes...>(in, out);
  }

  template <typename TOut, Index N, typename TRasterIn>
  VecRaster<TOut, N> correlateSamples(const TRasterIn& in, const PositionSampling<N>& sampling) const {
    VecRaster<TOut, N> out(sampling.shape());
    correlateSamplesTo(in, sampling, out);
  }

  template <typename TRasterIn, Index N, typename TRasterOut>
  void correlateSamplesTo(const TRasterIn& in, const PositionSampling<N>& sampling, TRasterOut& out) const {
    correlateSamplesAlongSeqTo<TRasterIn, N, TRasterOut, Axes...>(in, sampling, out);
  }

private:
  template <typename TRasterIn, typename TRasterOut, Index I0, Index... Is>
  void correlateAlongSeqTo(const TRasterIn& in, TRasterOut& out) const {
    const auto tmp = correlateAlong<TRasterIn, TRasterOut, I0>(in);
    correlateAlongSeqTo<TRasterIn, TRasterOut, Is...>(tmp, out);
    printf("%li: %i -> %i\n", I0, tmp[0], out[0]);
  }

  template <typename TRasterIn, typename TRasterOut>
  void correlateAlongSeqTo(const TRasterIn& in, TRasterOut& out) const {
    // pass
  }

  template <typename TRasterIn, typename TRasterOut, Index I>
  TRasterOut correlateAlong(const TRasterIn& in) const {
    const auto shape = in.shape();
    const auto length = shape[I];
    const auto stride = shapeStride<I>(shape);
    auto domain = in.domain();
    domain.back[I] = 0;
    TRasterOut out(shape);
    for (const auto& p : domain) {
      DataSamples<const typename TRasterIn::Value> inSamples {&in[p], length, {}, stride};
      DataSamples<typename TRasterOut::Value> outSamples {&out[p], length, {}, stride};
      m_kernels.at(I)->correlate(inSamples, outSamples);
    }
    return out;
  }

  template <typename TRasterIn, Index N, typename TRasterOut, Index I0, Index... Is>
  void correlateSamplesAlongSeqTo(const TRasterIn& in, const PositionSampling<N>& sampling, TRasterOut& out) const {
    auto box = correlationBox(sampling, in.shape()); // FIXME make unit and dilate
    correlateAlongSeqTo<TRasterIn, N, TRasterOut, Is...>(in, box, out);
    // FIXME m_kernels[I0]->correlateSamplesAlongTo<I0>(in, sampling, out)
  }

  template <Index N>
  Region<N> correlationBox(const PositionSampling<N>& sampling, const Position<N>& shape) const {
    auto box = sampling.region();
    for (const auto& p : m_kernels) {
      auto& front = box.front[p.first];
      front -= p.second.backwardCount();
      if (front < 0) {
        front = 0;
      }
      auto& back = box.back[p.first];
      back += p.second.forwardCount();
      if (back >= shape[p.first]) {
        back = shape[p.first] - 1;
      }
    }
    return box;
  }

private:
  std::map<Index, const TKernel*> m_kernels;
};

} // namespace Kast

#endif
