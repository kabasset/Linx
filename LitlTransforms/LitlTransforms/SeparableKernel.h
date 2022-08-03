// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_SEPARABLEKERNEL_H
#define _LITLTRANSFORMS_SEPARABLEKERNEL_H

#include "LitlRaster/Raster.h"
#include "LitlTransforms/Kernel.h"
#include "LitlTransforms/LineKernel.h"

#include <map>
#include <vector>

namespace Litl {

/**
 * @brief Separable correlation kernel as a sequence of oriented line kernels.
 */
template <typename T, Index I0, Index... Is>
class SeparableKernel {
  template <typename U, Index J0, Index... Js>
  friend class SeparableKernel;

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
   * @brief Construct a sequence of identical `LineKernel`s with various orientations.
   */
  explicit SeparableKernel(const LineKernel<T>& kernel) : m_kernels {{I0, kernel}, {Is, kernel}...} {}

  /**
   * @brief Map-based constructor.
   */
  template <typename... Ts>
  explicit SeparableKernel(Ts&&... args) : m_kernels(std::forward<Ts>(args)...) {}

  /**
   * @brief Make a Prewitt correlation kernel along given axes.
   * @see `sobel()`
   */
  static SeparableKernel prewitt(T sign = 1) {
    const auto derivation = LineKernel<T>({sign, 0, -sign}).template along<I0>();
    const auto averaging = LineKernel<T>({1, 1, 1}).template along<Is...>();
    return derivation * averaging;
  }

  /**
   * @brief Make a Sobel correlation kernel along given axes.
   * @param sign The differentiation sign (-1 or 1)
   * 
   * The kernel along the `Is` axes is `{1, 2, 1}` and that along `I0` is `{sign, 0, -sign}`.
   * Note the ordering of the differentiation _correlation_ kernel, which is opposite to Sobel's _convolution_ kernel.
   * For differenciation in the increasing-index direction, keep `sign = 1`;
   * for the opposite direction, set `sign = -1`.
   * 
   * For example, to compute the derivative along axis 1 backward, while averaging along axes 0 and 2, do:
   * \code
   * auto kernel = SeparableKernel<int, 1, 0, 2>::sobel(-1);
   * auto dy = kernel * raster;
   * \endcode
   */
  static SeparableKernel sobel(T sign = 1) {
    const auto derivation = LineKernel<T>({sign, 0, -sign}).template along<I0>();
    const auto averaging = LineKernel<T>({1, 2, 1}).template along<Is...>();
    return derivation * averaging;
  }

  /**
   * @brief Make a Scharr correlation kernel along given axes.
   * @see `sobel()`
   */
  static SeparableKernel<T, I0, Is...> scharr(T sign = 1) {
    const auto derivation = LineKernel<T>({sign, 0, -sign}).template along<I0>();
    const auto averaging = LineKernel<T>({3, 10, 3}).template along<Is...>();
    return derivation * averaging;
  }

  /**
   * @brief Make a separable Laplacian correlation kernel along given axes.
   * 
   * The kernel is built as a sequence of 1D kernels `{1, -2, 1}` if `sign` is 1,
   * or `{-1, 2, -1}` if sign is -1.
   */
  static SeparableKernel laplacian(T sign = 1) {
    return SeparableKernel(LineKernel<T>({sign, sign * -2, sign}));
  }

  /// @group_properties

  /**
   * @brief The logical window of the kernel.
   */
  Box<Dimension> window() const {
    Box<Dimension> box(Position<Dimension>::zero(), Position<Dimension>::zero());
    for (const auto& k : m_kernels) {
      auto i = k.first;
      const auto& w = k.second.window();
      if (w.front() < box[i]) {
        box[i] = w.front();
      }
      if (w.back() > box[i]) {
        box[i] = w.back();
      }
    }
    return box;
  }

  /**
   * @brief Get the `LineKernel` along given axis.
   */
  const T& operator[](Index axis) const {
    return m_kernels.at(axis);
  }

  /**
   * @brief Beginning iterator over `{Index, LineKernel}` pairs.
   */
  const decltype(auto) begin() const {
    return m_kernels.begin();
  }

  /**
   * @brief End iterator over `{Index, LineKernel}` pairs.
   */
  const decltype(auto) end() const {
    return m_kernels.end();
  }

  /**
   * @brief Convolve the separable components as a single ND kernel.
   */
  Kernel<Value, Dimension> compose() const {
    auto shape = Position<Dimension>::one();
    auto origin = Position<Dimension>::zero();
    for (const auto& k : m_kernels) {
      shape[k.first] = k.second.size();
      origin[k.first] = k.second.origin();
    }
    auto raster = Raster<Value, Dimension>(shape).fill(T(0));
    raster[origin] = T(1);
    return kernelize(*this * raster, origin);
  }

  /**
   * @brief Combine two sequences of kernels.
   */
  template <Index... Js>
  SeparableKernel<T, I0, Is..., Js...> operator*(const SeparableKernel<T, Js...>& rhs) const {
    SeparableKernel<T, I0, Is..., Js...> out(m_kernels);
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
      front -= p.second.origin();
      if (front < 0) {
        front = 0;
      }
      auto& back = box.back[p.first];
      back += p.second.size() - p.second.origin() - 1;
      if (back >= shape[p.first]) {
        back = shape[p.first] - 1;
      }
    }
    return box; //FIXME box.grow(amount).clamp(shape)
  }

private:
  std::map<Index, LineKernel<T>> m_kernels;
};

} // namespace Litl

#endif
