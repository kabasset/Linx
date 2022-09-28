// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_SEPARABLEKERNEL_H
#define _LITLTRANSFORMS_SEPARABLEKERNEL_H

#include "LitlCore/Raster.h"
#include "LitlTransforms/Extrapolation.h"
#include "LitlTransforms/Kernel.h"
#include "LitlTransforms/OrientedKernel.h"
#include "LitlTypes/SeqUtils.h"

#include <type_traits> // decay

namespace Litl {

/**
 * @brief Separable correlation kernel as a sequence of oriented 1D kernels.
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
   * @brief Construct a sequence of identical kernels with various orientations.
   */
  explicit SeparableKernel(const std::vector<T>& values) : SeparableKernel(values, (values.size() - 1) / 2) {}

  /**
   * @brief Construct a sequence of identical kernels with various orientations.
   */
  explicit SeparableKernel(const std::vector<T>& values, Index origin) :
      m_kernels {OrientedKernel<T, I0> {values, origin}, OrientedKernel<T, Is> {values, origin}...} {}

  /**
   * @brief Tuple-based constructor.
   */
  // template <typename... Ts>
  // explicit SeparableKernel(Ts&&... args) : m_kernels(std::forward<Ts>(args)...) {}
  explicit SeparableKernel(std::tuple<OrientedKernel<T, I0>, OrientedKernel<T, Is>...> kernels) :
      m_kernels(std::move(kernels)) {}

  /**
   * @brief Make a Prewitt correlation kernel along given axes.
   * @see `sobel()`
   */
  static SeparableKernel prewitt(T sign = 1) {
    const auto derivation = OrientedKernel<T, I0>({-sign, 0, sign});
    const auto averaging = SeparableKernel<T, Is...>({1, 1, 1});
    return derivation * averaging;
  }

  /**
   * @brief Make a Sobel correlation kernel along given axes.
   * @param sign The differentiation sign (-1 or 1)
   * 
   * The kernel along the `Is` axes is `{1, 2, 1}` and that along `I0` is `{-sign, 0, sign}`.
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
    const auto derivation = OrientedKernel<T, I0>({-sign, 0, sign});
    const auto averaging = SeparableKernel<T, Is...>({1, 2, 1});
    return derivation * averaging;
  }

  /**
   * @brief Make a Scharr correlation kernel along given axes.
   * @see `sobel()`
   */
  static SeparableKernel<T, I0, Is...> scharr(T sign = 1) {
    const auto derivation = OrientedKernel<T, I0>({-sign, 0, sign});
    const auto averaging = SeparableKernel<T, Is...>({3, 10, 3});
    return derivation * averaging;
  }

  /**
   * @brief Make a separable Laplacian correlation kernel along given axes.
   * 
   * The kernel is built as a sequence of 1D kernels `{1, -2, 1}` if `sign` is 1,
   * or `{-1, 2, -1}` if sign is -1.
   */
  static SeparableKernel<T, I0, Is...> laplacian(T sign = 1) {
    return SeparableKernel<T, I0, Is...>({sign, sign * -2, sign});
  }

  /// @group_properties

  /**
   * @brief The logical window of the kernel.
   */
  Box<Dimension> window() const {
    auto front = Position<Dimension>::zero();
    auto back = Position<Dimension>::zero();
    seqForeach(m_kernels, [&](const auto& k) {
      front[k.Axis] = std::min(front[k.Axis], k.window().front());
      back[k.Axis] = std::max(back[k.Axis], k.window().back());
    });
    return {front, back};
  }

  /**
   * @brief Convolve the separable components as a single ND kernel.
   */
  Kernel<Value, Dimension> compose() const {
    const auto w = window();
    const auto o = -w.front();
    auto raster = Raster<Value, Dimension>(w.shape());
    raster[o] = T(1); // FIXME or back-o?
    auto impulse = *this * extrapolate(raster, 0);
    return kernelize(impulse.reverse(), o);
  }

  /**
   * @brief Combine two sequences of kernels.
   */
  template <Index... Js>
  SeparableKernel<T, I0, Is..., Js...> operator*(const SeparableKernel<T, Js...>& rhs) const {
    return SeparableKernel<T, I0, Is..., Js...>(std::tuple_cat(m_kernels, rhs.m_kernels));
  }

  template <Index J>
  SeparableKernel<T, I0, Is..., J> operator*(const OrientedKernel<T, J>& rhs) const {
    return SeparableKernel<T, I0, Is..., J>(std::tuple_cat(m_kernels, rhs));
  }

  template <Index J>
  friend SeparableKernel<T, J, I0, Is...>
  operator*(const OrientedKernel<T, J>& lhs, const SeparableKernel<T, I0, Is...>& rhs) {
    return SeparableKernel<T, J, I0, Is...>(std::tuple_cat(std::make_tuple(lhs), rhs.m_kernels));
  }

  /**
   * @brief Apply the correlation kernels to an input extrapolator.
   */
  template <typename TIn>
  Raster<std::decay_t<typename TIn::Value>, TIn::Dimension> operator*(const TIn& in) const {
    Raster<std::decay_t<typename TIn::Value>, TIn::Dimension> out(in.shape());
    correlateTo(in, out);
    return out;
  }

  /**
   * @copydoc correlate()
   */
  template <typename TIn, typename TOut>
  void correlateTo(const TIn& in, TOut& out) const {
    correlateKernelSeq<TIn, TOut, I0, Is...>(in, out);
  }

private:
  template <typename TIn, typename TOut, Index J0, Index... Js>
  void correlateKernelSeq(const TIn& in, TOut& out) const {
    const auto tmp = correlateKthKernel<TIn, TOut, sizeof...(Is) - sizeof...(Js)>(in);
    const auto& method = in.method();
    const Extrapolator<TOut, decltype(method)> extrapolator(tmp, method);
    correlateKernelSeq<decltype(extrapolator), TOut, Js...>(extrapolator, out);
  }

  template <typename TIn, typename TOut>
  void correlateKernelSeq(const TIn& in, TOut& out) const {
    out = rasterize(in); // FIXME swap? move?
  }

  template <typename TIn, typename TOut, std::size_t K>
  TOut correlateKthKernel(const TIn& in) const {
    return std::get<K>(m_kernels) * in;
  }

private:
  std::tuple<OrientedKernel<T, I0>, OrientedKernel<T, Is>...> m_kernels;
};

template <typename U, Index J0, Index J1>
SeparableKernel<U, J0, J1> operator*(OrientedKernel<U, J0> lhs, OrientedKernel<U, J1> rhs) {
  return SeparableKernel<U, J0, J1>(std::make_tuple(lhs, rhs));
}

} // namespace Litl

#endif
