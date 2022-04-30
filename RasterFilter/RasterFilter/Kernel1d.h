// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERFILTER_KERNEL1D_H
#define _RASTERFILTER_KERNEL1D_H

#include "Raster/Raster.h"
#include "Raster/Sampling.h"
#include "RasterGeometry/Extrapolation.h"

#include <map>
#include <vector>

namespace Cnes {

/// @cond
template <typename TKernel, Index... Is>
class Kernel1dSeq;
/// @endcond

/**
 * @brief 1D kernel for nD correlations.
 */
template <typename T, typename TExtrapolation = CropExtrapolation>
class Kernel1d : public DataContainer<T, DataContainerHolder<T, std::vector<T>>, Kernel1d<T, TExtrapolation>> {

public:
  using Value = T;
  using Extrapolation = TExtrapolation;

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
  Index backwardSize() const {
    return m_backward;
  }

  /**
   * @brief Get the number of forward values.
   */
  Index forwardSize() const {
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
  template <Index... Is>
  Kernel1dSeq<Kernel1d<T, TExtrapolation>, Is...> along() const {
    return Kernel1dSeq<Kernel1d<T, TExtrapolation>, Is...>(*this);
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
    auto i = in.front();

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
template <typename TKernel, Index... Is>
class Kernel1dSeq {
  template <typename UKernel, Index... Js>
  friend class Kernel1dSeq;

public:
  /**
   * @brief The element value type.
   */
  using Value = typename TKernel::Value;

  /**
   * @brief The logical dimension of the combined kernel.
   */
  static constexpr Index Dim = std::max({Is...}) + 1;

  /**
   * @brief Construct a sequence of identical `Kernel1d`s with various orientations.
   */
  explicit Kernel1dSeq(const Kernel1d<typename TKernel::Value, typename TKernel::Extrapolation>& kernel) :
      m_kernels {{Is, kernel}...} {}

  /**
   * @brief Map-based constructor.
   */
  template <typename... Ts>
  explicit Kernel1dSeq(Ts&&... args) : m_kernels(std::forward<Ts>(args)...) {}

  /**
   * @brief Get the `Kernel1d` along given axis.
   */
  const TKernel& operator[](Index axis) const {
    return m_kernels.at(axis);
  }

  /**
   * @brief Beginning iterator over `{Index, Kernel1d}` pairs.
   */
  const decltype(auto) begin() const {
    return m_kernels.begin();
  }

  /**
   * @brief End iterator over `{Index, Kernel1d}` pairs.
   */
  const decltype(auto) end() const {
    return m_kernels.end();
  }

  /**
   * @brief Combine the separable components as a nD kernel.
   */
  VecRaster<Value, Dim> combine() const { // FIXME return Kernel<Value, Dim>
    auto shape = Position<Dim>::one();
    auto origin = Position<Dim>::zero();
    for (const auto& k : m_kernels) {
      shape[k.first] = k.second.size();
      origin[k.first] = k.second.backwardSize();
    }
    VecRaster<Value, Dim> raster(shape);
    // FIXME
    return raster;
  }

  /**
   * @brief Combine two sequences of kernels.
   */
  template <Index... Js>
  Kernel1dSeq<TKernel, Is..., Js...> operator*(const Kernel1dSeq<TKernel, Js...>& rhs) const {
    Kernel1dSeq<TKernel, Is..., Js...> out(m_kernels);
    out.m_kernels.insert(rhs.begin(), rhs.end());
    return out;
  }

  /**
   * @brief Apply the correlation kernels to an input raster.
   */
  template <typename TOut, typename TIn, Index N, typename TContainer>
  VecRaster<TOut, N> correlate(const Raster<TIn, N, TContainer>& in) const {
    VecRaster<TOut, N> out(in.shape());
    correlateTo(in, out);
    return out;
  }

  /**
   * @copydoc correlate()
   */
  template <typename TRasterIn, typename TRasterOut>
  void correlateTo(const TRasterIn& in, TRasterOut& out) const {
    correlateAlongSeqTo<TRasterIn, TRasterOut, Is...>(in, out);
  }

  /**
   * @brief Sparsely apply the correlation kernels to an input raster.
   */
  template <typename TOut, Index N, typename TRasterIn>
  VecRaster<TOut, N> correlateSamples(const TRasterIn& in, const PositionSampling<N>& sampling) const {
    VecRaster<TOut, N> out(sampling.shape());
    correlateSamplesTo(in, sampling, out);
  }

  /**
   * @copydoc correlateSamples()
   */
  template <typename TRasterIn, Index N, typename TRasterOut>
  void correlateSamplesTo(const TRasterIn& in, const PositionSampling<N>& sampling, TRasterOut& out) const {
    correlateSamplesAlongSeqTo<TRasterIn, N, TRasterOut, Is...>(in, sampling, out);
  }

private:
  template <typename TRasterIn, typename TRasterOut, Index J0, Index... Js>
  void correlateAlongSeqTo(const TRasterIn& in, TRasterOut& out) const {
    const auto tmp = correlateAlong<TRasterIn, TRasterOut, J0>(in);
    correlateAlongSeqTo<TRasterOut, TRasterOut, Js...>(tmp, out);
    printf("%li: %i -> %i\n", J0, tmp[0], out[0]);
  }

  template <typename TRasterIn, typename TRasterOut>
  void correlateAlongSeqTo(const TRasterIn& in, TRasterOut& out) const {
    out = in; // FIXME swap? move?
  }

  template <typename TRasterIn, typename TRasterOut, Index J>
  TRasterOut correlateAlong(const TRasterIn& in) const {
    const auto shape = in.shape();
    const auto length = shape[J];
    const auto stride = shapeStride<J>(shape);
    auto domain = in.domain();
    domain.back[J] = 0;
    TRasterOut out(shape);
    for (const auto& p : domain) {
      DataSamples<const typename TRasterIn::Value> inSamples {&in[p], length, {}, stride};
      DataSamples<typename TRasterOut::Value> outSamples {&out[p], length, {}, stride};
      m_kernels.at(J).correlate(inSamples, outSamples);
    }
    return out;
  }

  template <typename TRasterIn, Index N, typename TRasterOut, Index J0, Index... Js>
  void correlateSamplesAlongSeqTo(const TRasterIn& in, const PositionSampling<N>& sampling, TRasterOut& out) const {
    auto box = correlationBox(sampling, in.shape()); // FIXME make unit and dilate
    correlateAlongSeqTo<TRasterIn, N, TRasterOut, Js...>(in, box, out);
    // FIXME m_kernels[J0]->correlateSamplesAlongTo<J0>(in, sampling, out)
  }

  template <Index N>
  Region<N> correlationBox(const PositionSampling<N>& sampling, const Position<N>& shape) const {
    auto box = sampling.region();
    for (const auto& p : m_kernels) {
      auto& front = box.front[p.first];
      front -= p.second.backwardSize();
      if (front < 0) {
        front = 0;
      }
      auto& back = box.back[p.first];
      back += p.second.forwardSize();
      if (back >= shape[p.first]) {
        back = shape[p.first] - 1;
      }
    }
    return box; //FIXME box.dilate(amount).clamp(shape)
  }

private:
  std::map<Index, TKernel> m_kernels;
};

/***
 * @brief Make a Sobel correlation kernel along given axes.
 * @details
 * The kernel along the `IAverage` axis is `{1, 2, 1}` and that along `IDifference` is `{1, 0, -1}`.
 * Note the ordering of the differentiation kernel, which is opposite to Sobel's _convolution_ kernel.
 */
template <typename T, Index IDifference, Index IAverage, typename TExtrapolation = CropExtrapolation>
Kernel1dSeq<Kernel1d<T, TExtrapolation>, IDifference, IAverage> makeSobel() {
  return Kernel1d<T, TExtrapolation>({1, 0, -1}, 1).template along<IDifference>() *
      Kernel1d<T, TExtrapolation>({1, 2, 1}, 1).template along<IAverage>();
}

// FIXME
// isCorrelation<T>()
// decltype(auto) operator* (enable_if_t<isCorrelation<TKernel>>(), TRaster)

} // namespace Cnes

#endif
