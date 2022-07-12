// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_LINEKERNEL_H
#define _LITLTRANSFORMS_LINEKERNEL_H

#include "LitlRaster/Raster.h"
#include "LitlRaster/Sampling.h"
#include "LitlTransforms/Interpolation.h"
#include "LitlTypes/Segment.h"

#include <map>
#include <vector>

namespace Litl {

/// @cond
template <typename T, Index I0, Index... Is>
class SeparableKernel;
/// @endcond

/**
 * @brief 1D kernel for ND correlations.
 */
template <typename T>
class LineKernel : public DataContainer<T, StdHolder<std::vector<T>>, VectorArithmetic, LineKernel<T>> {

public:
  using Value = T;
  using Container = DataContainer<T, StdHolder<std::vector<T>>, VectorArithmetic, LineKernel<T>>;

  /**
   * @brief Constructor.
   * @param values The kernel values
   * @param origin The index of the kernel origin
   */
  explicit LineKernel(std::vector<T> values, Index origin) : Container(std::move(values)), m_origin(origin), m_bias() {}

  /**
   * @brief Constructor.
   */
  explicit LineKernel(std::vector<T> values) : LineKernel(std::move(values), values.size() / 2) {}

  /**
   * @brief Get the window.
   */
  Segment window() const {
    return Segment::fromSize(-m_origin, this->size());
  }

  /**
   * @brief Get the index of the origin.
   */
  Index origin() const {
    return m_origin;
  }

  /**
   * @brief Get a pointer to the data at origin.
   */
  const T* originData() const {
    return this->data() + m_origin;
  }

  /**
   * @brief Orient the kernel along given axes.
   * @details
   * For example, to apply a correlation kernel along axes 1 and 2 of a given raster `in`, do:
   * \code
   * auto out = kernel.template along<1, 2>().correlate(in);
   * \endcode
   */
  template <Index... Is>
  SeparableKernel<T, Is...> along() const {
    return SeparableKernel<T, Is...>(*this);
  }

  /**
   * @brief Correlate a given sampled data with the kernel.
   */
  template <typename TIn, typename TOut>
  void correlate(const DataSamples<TIn>& in, DataSamples<TOut>& out) const {
    // FIXME only valid for "kernel croping" extrapolation

    // Set up iterators
    const auto step = in.step();
    DataSamples<const TIn>
        unitIn {in.data(), in.size(), {in.front(), in.back()}, in.stride()}; // step = 1 for inner_product
    auto inIt = unitIn.begin();
    auto inMinIt = inIt;
    inMinIt -= in.front();
    inIt -= m_origin;
    auto outIt = out.begin();
    auto i = in.front();

    // Backward-croped
    for (; i < m_origin; i += step, inIt += step, ++outIt) {
      *outIt = std::inner_product(originData() - i, this->end(), inMinIt, m_bias);
    }

    // Central
    const auto forward = this->size() - m_origin - 1;
    for (; i <= in.size() - forward - step; i += step, inIt += step, ++outIt) {
      *outIt = std::inner_product(this->begin(), this->end(), inIt, m_bias);
    }

    // Forward-croped
    for (; i <= in.back(); i += step, inIt += step, ++outIt) {
      *outIt = std::inner_product(this->begin(), originData() + (in.size() - i), inIt, m_bias);
    }
  }

private:
  Index m_origin;
  T m_bias;
};

} // namespace Litl

#endif
