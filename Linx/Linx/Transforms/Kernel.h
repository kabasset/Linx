// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_KERNEL_H
#define _LINXTRANSFORMS_KERNEL_H

#include "Linx/Base/TypeUtils.h"
#include "Linx/Transforms/StructuringElement.h"
#include "Linx/Transforms/impl/Filter.h"

namespace Linx {

/**
 * @brief Kernel-based operations.
 */
enum class KernelOp {
  Convolution, ///< Convolution
  Correlation ///< Cross-correlation
};

/**
 * @brief A kernel which can be used for convolution or cross-correlation.
 */
template <KernelOp TOp, typename T, Index N = 2>
class Kernel : public FilterMixin<T, Box<N>, Kernel<TOp, T, N>> {
public:

  using typename FilterMixin<T, Box<N>, Kernel<TOp, T, N>>::Value;
  using FilterMixin<T, Box<N>, Kernel<TOp, T, N>>::Dimension;

  /**
   * @brief Constructor.
   */
  Kernel(const Value* values, Box<Dimension> window) :
      FilterMixin<T, Box<N>, Kernel<TOp, T, N>>(std::move(window)), m_values(values, values + this->m_window.size())
  {}

  /**
   * @brief View the kernel values as a `Raster`.
   */

  PtrRaster<const Value, Dimension> raster() const
  {
    return PtrRaster<const Value, Dimension>(this->window().shape(), m_values.data());
  }

  /**
   * @copybrief raster()const
   */
  PtrRaster<Value, Dimension> raster()
  {
    return PtrRaster<Value, Dimension>(this->window().shape(), m_values.data());
  }

  /**
   * @brief Estimation operator.
   */
  template <typename TIn>
  inline Value operator()(const TIn& neighbors) const
  { // FIXME use StructuringElement design
    if constexpr (TOp == KernelOp::Correlation) {
      return std::inner_product(m_values.begin(), m_values.end(), neighbors.begin(), Value {});
      // FIXME conjugate for complex values
    }
    if constexpr (TOp == KernelOp::Convolution) {
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
 * @brief Correlation operation.
 */
template <typename T>
class Correlation {
public:

  /**
   * @brief The correlation type.
   */
  using Value = T;

  /**
   * @brief Constructor.
   */
  template <typename TRange>
  Correlation(TRange&& values) : m_kernel(std::move(values))
  {
    if constexpr (is_complex<T>()) {
      for (auto& e : m_kernel) {
        e = std::conj(e);
      }
    }
  }

  /**
   * @brief Perform the operation on given neighborhood values.
   */
  template <typename TIn>
  inline Value operator()(const TIn& neighbors) const
  {
    return std::inner_product(m_kernel.begin(), m_kernel.end(), neighbors.begin(), Value {});
  }

private:

  /**
   * @brief The kernel.
   */
  std::vector<T> m_kernel;
};

/**
 * @brief Convolution operation.
 */
template <typename T>
class Convolution {
public:

  /**
   * @brief The convolution type.
   */
  using Value = T;

  /**
   * @brief Constructor.
   */
  template <typename TRange>
  Convolution(TRange&& values) : m_kernel(std::move(values))
  {}

  /**
   * @brief Perform the operation on given neighborhood values.
   */
  template <typename TIn>
  inline Value operator()(const TIn& neighbors) const
  {
    return std::inner_product(m_kernel.rbegin(), m_kernel.rend(), neighbors.begin(), Value {});
  }

private:

  /**
   * @brief The kernel.
   */
  std::vector<T> m_kernel;
};

/**
 * @relatesalso Kernel
 * @brief Make a convolution kernel from values and a window.
 */
template <typename T, Index N = 2>
auto convolution(const T* values, Box<N> window)
{
  const T* end = values + window.size();
  return StructuringElement<Convolution<T>, Box<N>>(std::vector<T>(values, end), std::move(window));
}

/**
 * @relatesalso Kernel
 * @brief Make a convolution kernel from a raster and origin position.
 */
template <typename T, Index N, typename THolder>
auto convolution(const Raster<T, N, THolder>& values, Position<N> origin)
{
  return convolution(values.data(), values.domain() - origin);
}

/**
 * @relatesalso Kernel
 * @brief Make a convolution kernel from a raster, with centered origin.
 * 
 * In case of even lengths, origin position is rounded down.
 */
template <typename T, Index N, typename THolder>
auto convolution(const Raster<T, N, THolder>& values)
{
  return convolution(values.data(), values.domain() - (values.shape() - 1) / 2);
}

/**
 * @relatesalso Kernel
 * @brief Make a correlation kernel from values and a window.
 */
template <typename T, Index N = 2>
auto correlation(const T* values, Box<N> window)
{
  const T* end = values + window.size();
  return StructuringElement<Correlation<T>, Box<N>>(std::vector(values, end), std::move(window));
}

/**
 * @relatesalso Kernel
 * @brief Make a correlation kernel from a raster and origin position.
 */
template <typename T, Index N, typename THolder>
auto correlation(const Raster<T, N, THolder>& values, Position<N> origin)
{
  return correlation(values.data(), values.domain() - origin);
}

/**
 * @relatesalso Kernel
 * @brief Make a correlation kernel from a raster, with centered origin.
 * 
 * In case of even lengths, origin position is rounded down.
 */
template <typename T, Index N, typename THolder>
auto correlation(const Raster<T, N, THolder>& values)
{
  return correlation(values.data(), values.domain() - (values.shape() - 1) / 2);
}

} // namespace Linx

#endif
