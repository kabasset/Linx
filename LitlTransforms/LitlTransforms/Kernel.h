// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_KERNEL_H
#define _LITLTRANSFORMS_KERNEL_H

#include "LitlTransforms/impl/Filter.h"

namespace Litl {

enum class KernelType { // FIXME KernelTag?
  Convolution,
  Correlation
};

/**
 * @brief A kernel which can be used for convolution or cross-correlation.
 */
template <KernelType TKernel, typename T, Index N = 2>
class Kernel : public FilterMixin<T, Box<N>, Kernel<TKernel, T, N>> {

public:
  using typename FilterMixin<T, Box<N>, Kernel<TKernel, T, N>>::Value;
  using FilterMixin<T, Box<N>, Kernel<TKernel, T, N>>::Dimension;

  /**
   * @brief Constructor.
   */
  Kernel(const Value* values, Box<Dimension> window) :
      FilterMixin<T, Box<N>, Kernel<TKernel, T, N>>(std::move(window)),
      m_values(values, values + this->m_window.size()) {}

  /**
   * @brief View the kernel values as a `Raster`.
   */

  PtrRaster<const Value, Dimension> raster() const {
    return PtrRaster<const Value, Dimension>(this->window().shape(), m_values.data());
  }

  /**
   * @copybrief raster()
   */
  PtrRaster<Value, Dimension> raster() {
    return PtrRaster<Value, Dimension>(this->window().shape(), m_values.data());
  }

  /**
   * @brief Estimation operator.
   */
  template <typename TIn>
  Value operator()(const TIn& neighbors) const {
    if constexpr (TKernel == KernelType::Correlation) {
      return std::inner_product(m_values.begin(), m_values.end(), neighbors.begin(), Value {});
    }
    if constexpr (TKernel == KernelType::Convolution) {
      return std::inner_product(m_values.rbegin(), m_values.rend(), neighbors.begin(), Value {});
    }
  }

private:
  /**
   * @brief The correlation values.
   */
  std::vector<Value> m_values;
};

/**
 * @relates Kernel
 * @brief Make a convolution kernel from values and a window.
 */
template <typename T, Index N = 2>
Kernel<KernelType::Convolution, T, N> convolution(const T* values, Box<N> window) {
  return Kernel<KernelType::Convolution, T, N>(values, std::move(window));
}

/**
 * @relates Kernel
 * @brief Make a kernel from a raster and origin position.
 */
template <typename T, Index N, typename THolder>
Kernel<KernelType::Convolution, T, N> convolution(const Raster<T, N, THolder>& values, Position<N> origin) {
  return convolution(values.data(), values.domain() - origin);
}

/**
 * @relates Kernel
 * @brief Make a kernel from a raster, with centered origin.
 * 
 * In case of even lengths, origin position is rounded down.
 */
template <typename T, Index N, typename THolder>
Kernel<KernelType::Convolution, T, N> convolution(const Raster<T, N, THolder>& values) {
  return convolution(values.data(), values.domain() - (values.shape() - 1) / 2);
}

} // namespace Litl

#endif
