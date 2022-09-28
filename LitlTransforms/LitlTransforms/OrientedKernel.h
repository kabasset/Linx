// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_ORIENTEDKERNEL_H
#define _LITLTRANSFORMS_ORIENTEDKERNEL_H

#include "LitlRaster/Raster.h"
#include "LitlTransforms/Extrapolation.h"
#include "LitlTypes/Slice.h"

#include <type_traits> // decay
#include <vector>

namespace Litl {

/**
 * @brief An axis-aligned 1D kernel.
 */
template <typename T, Index I>
class OrientedKernel {
public:
  /**
   * @brief The index of the axis the kernel is aligned to.
   */
  static constexpr Index Axis = I;

  /**
   * @brief Constructor.
   */
  OrientedKernel(std::vector<T> values, Index origin) : m_values(std::move(values)), m_origin(origin) {}

  /**
   * @brief Create a kernel with centered origin.
   * 
   * If the number of values is even, then the origin index is rounded down.
   */
  OrientedKernel(std::vector<T> values) : OrientedKernel(values, (values.size() - 1) / 2) {}

  /**
   * @brief Get the origin index.
   */
  Index origin() const {
    return m_origin;
  }

  /**
   * @brief Get the kernel window.
   */
  Slice window() const {
    return Slice::fromSize(-m_origin, m_values.size());
  }

  /**
   * @brief Correlate an input extrapolator.
   */
  template <typename TExtrapolator>
  Raster<std::decay_t<typename TExtrapolator::Value>, TExtrapolator::Dimension>
  operator*(const TExtrapolator& in) const {
    Raster<std::decay_t<typename TExtrapolator::Value>, TExtrapolator::Dimension> out(in.shape());
    correlateTo(in, out, out.domain());
    return out;
  }

  /**
   * @brief Correlate an input raster or extrapolator over a specified region.
   */
  template <typename TIn, typename TOut, typename TRegion>
  void correlateTo(const TIn& in, TOut& out, const TRegion region) const {
    auto front = Position<TIn::Dimension>::zero();
    front[I] -= m_origin;
    auto patch = in.patch(OrientedSlice<I, TIn::Dimension>::fromSize(std::move(front), m_values.size()));
    for (const auto& p : region) {
      patch.shift(p); // FIXME patch.moveBy(p)
      out[p] = std::inner_product(m_values.begin(), m_values.end(), patch.begin(), T {});
      // FIXME use out.patch(region) iterator?
      patch.shiftBack(p); // FIXME patch.moveAt(front);
    }
  }

private:
  /**
   * @brief The kernel values.
   */
  std::vector<T> m_values;

  /**
   * @brief The origin index.
   */
  Index m_origin;
};

} // namespace Litl

#endif
