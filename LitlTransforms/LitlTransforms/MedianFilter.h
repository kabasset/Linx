// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_KERNEL_H
#define _LITLTRANSFORMS_KERNEL_H

#include "LitlRaster/Raster.h"
#include "LitlTransforms/Interpolation.h"

namespace Litl {

/**
 * @brief A median filter.
 * @tparam T The value type
 * @tparam N The dimension
 * @tparam TBoundary The boundary conditions
 */
template <typename T, Index N>
class MedianFilter {

public:
  /**
   * @brief The value type.
   */
  using Value = T;

  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = N;

  /**
   * @brief Explcit window wonstructor.
   * @param window The filter window
   */
  MedianFilter(const Region<N>& window) : m_window(window) {}

  /**
   * @brief Radius-based constructor.
   * @param radius The filter radius
   * @details
   * The window is centered, and its lengths along all axes are the same.
   */
  MedianFilter(Index radius = 1) : m_window(Region<N>::fromCenter(radius)) {}

  /**
   * @brief Apply the filter.
   */
  template <typename TIn>
  Raster<typename TIn::Value, TIn::Dimension> apply(const TIn& in) {
    Raster<typename TIn::Value, TIn::Dimension> out(in.shape());
    applyTo(in, out, out.domain());
    return out;
  }

  /**
   * @brief Apply the filter into a given output raster.
   * @param in The input raster or an extrapolator
   * @param out The output raster
   * @param region The region to filter in the input raster
   */
  template <typename TIn, typename TOut>
  void applyTo(const TIn& in, TOut& out, const Region<N>& region = Region<N>::whole()) {
    std::vector<typename TIn::Value> neighbors(m_window.size());
    auto it = neighbors.begin();
    for (const auto& p : region) {
      for (const auto& q : m_window + p) { // FIXME dangling?
        *it++ = in.at(q);
      }
      out[p] = median<typename TOut::Value>(neighbors);
      it = neighbors.begin();
    }
  }

  /**
   * @brief Compute the median value of an iterable.
   * @warning
   * The iterable is modified (values are partially sorted).
   */
  template <typename U, typename TIterable>
  static U median(TIterable& through) {
    auto b = through.begin();
    auto e = through.end();
    const auto size = std::distance(b, e);
    auto n = b + size / 2;
    std::nth_element(b, n, e);
    if (size % 2 == 1) {
      return *n;
    }
    std::nth_element(b, n + 1, e);
    return (*n + *(n + 1)) * .5;
  }

private:
  /**
   * @brief The window.
   */
  Region<N> m_window;
};

} // namespace Litl

#endif
