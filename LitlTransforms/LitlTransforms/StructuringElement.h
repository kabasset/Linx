// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_STRUCTURINGELEMENT_H
#define _LITLTRANSFORMS_STRUCTURINGELEMENT_H

#include "LitlRaster/Raster.h"
#include "LitlTransforms/Interpolation.h"

namespace Litl {

/**
 * @brief A structuring element formorphological operations.
 * @tparam N The dimension
 */
template <typename T, Index N, typename TRegion = Box<N>>
class StructuringElement {

public:
  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = N;

  /**
   * @brief Explcit window constructor.
   * @param window The filter window
   */
  StructuringElement(const TRegion& window) : m_window(window), m_neighbors(m_window.size()) {}

  /**
   * @brief Radius-based constructor.
   * @param radius The filter radius
   * @details
   * The window is centered, and its lengths along all axes are the same.
   */
  StructuringElement(Index radius = 1) : StructuringElement(Box<N>::fromCenter(radius)) {}

  /**
   * @brief Apply the filter.
   */
  template <typename TIn>
  Raster<typename TIn::Value, TIn::Dimension> median(const TIn& in) {
    Raster<typename TIn::Value, TIn::Dimension> out(in.shape());
    medianTo(in, out, out.domain());
    return out;
  }

  /**
   * @brief Apply the filter into a given output raster.
   * @param in The input raster or an extrapolator
   * @param out The output raster
   * @param region The region to filter in the input raster
   */
  template <typename TIn, typename TOut>
  void medianTo(const TIn& in, TOut& out, const Box<N>& region = Box<N>::whole()) {
    for (const auto& p : region) {
      loadNeighbors(in, p);
      out[p] = neighborsMedian();
    }
  }

  template <typename TIn, typename TOut>
  void erodeTo(const TIn& in, TOut& out, const Box<N>& region = Box<N>::whole()) {
    for (const auto& p : region) {
      loadNeighbors(in, p);
      out[p] = neighborsMin();
    }
  }

  template <typename TIn, typename TOut>
  void dilateTo(const TIn& in, TOut& out, const Box<N>& region = Box<N>::whole()) {
    for (const auto& p : region) {
      loadNeighbors(in, p);
      out[p] = neighborsMax();
    }
  }

private:
  void loadNeighbors(const TIn& in, const Position<N>& p) {
    auto it = neighbors.begin();
    for (const auto& q : m_window + p) {
      *it++ = in[q];
    }
  }

  T neighborsMedian() {
    auto b = m_neighbors.begin();
    auto e = m_neighbors.end();
    const auto size = std::distance(b, e);
    auto n = b + size / 2;
    std::nth_element(b, n, e);
    if (size % 2 == 1) {
      return *n;
    }
    std::nth_element(b, n + 1, e);
    return (*n + *(n + 1)) * .5;
  }

  T neighborsMin() {
    return *std::min_element(m_neighbors.begin(), m_neighbors.end());
  }

  T neighborsMax() {
    return *std::max_element(m_neighbors.begin(), m_neighbors.end());
  }

  /**
   * @brief The window.
   */
  TRegion m_window;

  /**
   * @brief The neighboring pixel values.
   */
  std::vector<T> m_neighbors;
};

} // namespace Litl

#endif
