// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_KERNEL_H
#define _LITLTRANSFORMS_KERNEL_H

#include "LitlTransforms/impl/Correlator.h"

namespace Litl {

/**
 * @brief A kernel which can be used for convolution or cross-correlation.
 */
template <typename T, Index N = 2>
class Kernel : public CorrelatorMixin<T, N, Kernel<T, N>> { // FIXME DataContainer

public:
  using typename CorrelatorMixin<T, N, Kernel<T, N>>::Value;
  using CorrelatorMixin<T, N, Kernel<T, N>>::Dimension;

  /**
   * @brief Constructor.
   */
  Kernel(const T* values, Box<N> window) : m_values(values, values + window.size()), m_window(std::move(window)) {}

  /**
   * @brief Get the window.
   */
  const Box<Dimension>& window() const {
    return m_window;
  }

  /**
   * @brief View the kernel values as a `Raster`.
   */
  PtrRaster<const Value, Dimension> raster() const {
    return PtrRaster<const Value, Dimension>(m_window.shape(), m_values.data());
  }

  /**
   * @copybrief raster()
   */
  PtrRaster<Value, Dimension> raster() {
    return PtrRaster<Value, Dimension>(m_window.shape(), m_values.data());
  }

  /**
   * @brief Correlate an input raster or extrapolator over a given monolithic region.
   * @param in An input patch of raster or extrapolator
   * @param out An output raster or patch
   * 
   * The output domain must be compatible with the input domain.
   * Specifically, both domains will be iterated in parallel,
   * such that the result of the `n`-th correlation, at `std::advance(in.begin(), n)`,
   * will be written to `std::advance(out.begin(), n)`.
   * 
   * As opposed to other methods, no spatial optimization is performed:
   * the region is not sliced to isolate extrapolated values from non-extrapolated values.
   */
  template <typename TIn, typename TOut>
  void correlateMonolithTo(const TIn& in, TOut& out) const {
    auto window = in.parent().patch(m_window);
    auto outIt = out.begin();
    for (const auto& p : in.domain()) {
      window.translate(p);
      *outIt = std::inner_product(m_values.begin(), m_values.end(), window.begin(), T {});
      ++outIt;
      window.translateBack(p);
    }
  }

private:
  /**
   * @brief The correlation values.
   */
  std::vector<T> m_values;

  /**
   * @brief The correlation window.
   */
  Box<N> m_window;
};

/**
 * @relates Kernel
 * @brief Make a kernel from values and a window.
 */
template <typename T, Index N = 2>
Kernel<T, N> kernelize(const T* values, Box<N> window) {
  return Kernel<T, N>(values, std::move(window));
}

/**
 * @relates Kernel
 * @brief Make a kernel from a raster and origin position.
 */
template <typename T, Index N, typename THolder>
Kernel<T, N> kernelize(const Raster<T, N, THolder>& values, Position<N> origin) {
  return kernelize(values.data(), values.domain() - origin);
}

/**
 * @relates Kernel
 * @brief Make a kernel from a raster, with centered origin.
 * 
 * In case of even lengths, origin position is rounded down.
 */
template <typename T, Index N, typename THolder>
Kernel<T, N> kernelize(const Raster<T, N, THolder>& values) {
  return kernelize(values.data(), values.domain() - (values.shape() - 1) / 2);
}

} // namespace Litl

#endif
