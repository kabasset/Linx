// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_KERNEL1D_H
#define _LITLTRANSFORMS_KERNEL1D_H

#include "LitlRaster/Raster.h"
#include "LitlRaster/Sampling.h"
#include "LitlTransforms/Interpolation.h"

#include <map>
#include <vector>

namespace Litl {

/// @cond
template <typename T, Index I0, Index... Is>
class SepKernel;
/// @endcond

/**
 * @brief 1D kernel for nD correlations.
 */
template <typename T>
class Kernel1d : public DataContainer<T, StdHolder<std::vector<T>>, VectorArithmetic, Kernel1d<T>> {

public:
  using Value = T;

  /**
   * @brief Constructor.
   * @param values The kernel values
   * @param origin The index of the kernel origin
   */
  explicit Kernel1d(std::vector<T>&& values, Index origin) :
      DataContainer<T, StdHolder<std::vector<T>>, VectorArithmetic, Kernel1d<T>>(std::move(values)), m_backward(origin),
      m_forward(this->size() - 1 - m_backward), m_bias() {}

  /**
   * @brief Constructor.
   */
  explicit Kernel1d(std::vector<T>&& values) : Kernel1d(std::move(values), values.size() / 2) {}

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
  SepKernel<T, Is...> along() const {
    return SepKernel<T, Is...>(*this);
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
};

/**
 * @brief Separable correlation kernel as a sequence of oriented 1D kernels.
 */
template <typename T, Index I0, Index... Is>
class SepKernel {
  template <typename U, Index J0, Index... Js>
  friend class SepKernel;

public:
  /**
   * @brief The element value type.
   */
  using Value = T;

  /**
   * @brief The logical dimension of the combined kernel.
   */
  static constexpr Index Dimension = std::max({I0, Is...}) + 1;

  /**
   * @brief Construct a sequence of identical `Kernel1d`s with various orientations.
   */
  explicit SepKernel(const Kernel1d<T>& kernel) : m_kernels {{I0, kernel}, {Is, kernel}...} {}

  /**
   * @brief Map-based constructor.
   */
  template <typename... Ts>
  explicit SepKernel(Ts&&... args) : m_kernels(std::forward<Ts>(args)...) {}

  /**
   * @brief Make a Prewitt correlation kernel along given axes.
   * @see `sobel()`
   */
  static SepKernel prewitt(T sign = 1) {
    const auto derivation = Kernel1d<T>({sign, 0, -sign}).template along<I0>();
    const auto averaging = Kernel1d<T>({1, 1, 1}).template along<Is...>();
    return derivation * averaging;
  }

  /**
   * @brief Make a Sobel correlation kernel along given axes.
   * @param sign The differentiation sign (-1 or 1)
   * @details
   * The kernel along the `Is` axes is `{1, 2, 1}` and that along `I0` is `{sign, 0, -sign}`.
   * Note the ordering of the differentiation _correlation_ kernel, which is opposite to Sobel's _convolution_ kernel.
   * For differenciation in the increasing-index direction, keep `sign = 1`;
   * for the opposite direction, set `sign = -1`.
   * 
   * For example, to compute the derivative along axis 1 backward, while averaging along axes 0 and 2, do:
   * \code
   * auto kernel = SepKernel<int, 1, 0, 2>::sobel(-1);
   * auto dy = kernel * raster;
   * \endcode
   */
  static SepKernel sobel(T sign = 1) {
    const auto derivation = Kernel1d<T>({sign, 0, -sign}).template along<I0>();
    const auto averaging = Kernel1d<T>({1, 2, 1}).template along<Is...>();
    return derivation * averaging;
  }

  /**
   * @brief Make a Scharr correlation kernel along given axes.
   * @see `sobel()`
   */
  static SepKernel<T, I0, Is...> scharr(T sign = 1) {
    const auto derivation = Kernel1d<T>({sign, 0, -sign}).template along<I0>();
    const auto averaging = Kernel1d<T>({3, 10, 3}).template along<Is...>();
    return derivation * averaging;
  }

  /**
   * @brief Make a separable Laplacian correlation kernel along given axes.
   * @details
   * The kernel is built as a sequence of 1D kernels `{1, -2, 1}` if `sign` is 1,
   * or `{-1, 2, -1}` if sign is -1.
   */
  static SepKernel laplacian(T sign = 1) {
    return SepKernel(Kernel1d<T>({sign, sign * -2, sign}));
  }

  /**
   * @brief Get the `Kernel1d` along given axis.
   */
  const T& operator[](Index axis) const {
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
   * @brief Convolve the separable components as a single ND kernel.
   */
  Raster<Value, Dimension> compose() const { // FIMXE product()?
    auto shape = Position<Dimension>::one();
    auto origin = Position<Dimension>::zero();
    for (const auto& k : m_kernels) {
      shape[k.first] = k.second.size();
      origin[k.first] = k.second.backwardSize();
    }
    Raster<Value, Dimension> raster(shape);
    // FIXME
    return raster; // FIXME return Kernel<Value, Dimension>
  }

  /**
   * @brief Combine two sequences of kernels.
   */
  template <Index... Js>
  SepKernel<T, I0, Is..., Js...> operator*(const SepKernel<T, Js...>& rhs) const {
    SepKernel<T, I0, Is..., Js...> out(m_kernels);
    out.m_kernels.insert(rhs.begin(), rhs.end());
    return out;
  }

  /**
   * @brief Apply the correlation kernels to an input raster.
   */
  template <typename TIn, Index N, typename TContainer>
  Raster<T, N> operator*(const Raster<TIn, N, TContainer>& in) const {
    Raster<T, N> out(in.shape());
    correlateTo(in, out);
    return out;
  }

  /**
   * @copydoc correlate()
   */
  template <typename TRasterIn, typename TRasterOut>
  void correlateTo(const TRasterIn& in, TRasterOut& out) const {
    correlateAlongSeqTo<TRasterIn, TRasterOut, I0, Is...>(in, out);
  }

  /**
   * @brief Sparsely apply the correlation kernels to an input raster.
   */
  template <Index N, typename TRasterIn>
  Raster<T, N> correlateSamples(const TRasterIn& in, const PositionSampling<N>& sampling) const {
    Raster<T, N> out(sampling.shape());
    correlateSamplesTo(in, sampling, out);
  }

  /**
   * @copydoc correlateSamples()
   */
  template <typename TRasterIn, Index N, typename TRasterOut>
  void correlateSamplesTo(const TRasterIn& in, const PositionSampling<N>& sampling, TRasterOut& out) const {
    correlateSamplesAlongSeqTo<TRasterIn, N, TRasterOut, I0, Is...>(in, sampling, out);
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
    const auto domain = in.domain().project(J);
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
  Box<N> correlationBox(const PositionSampling<N>& sampling, const Position<N>& shape) const {
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
  std::map<Index, Kernel1d<T>> m_kernels;
};

} // namespace Litl

#endif
