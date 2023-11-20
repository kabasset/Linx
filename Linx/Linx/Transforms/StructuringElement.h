// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_STRUCTURINGELEMENT_H
#define _LINXTRANSFORMS_STRUCTURINGELEMENT_H

#include "Linx/Data/Raster.h"
#include "Linx/Transforms/Extrapolation.h"
#include "Linx/Transforms/impl/Filter.h"

namespace Linx {

/**
 * @brief Structuring element-based operations.
 */
namespace MorphologyOp {

/**
 * @brief Mean filtering.
 */
template <typename T>
struct MeanFilter {
  using Value = T; // FIXME deduce from operator()

  template <typename TIn>
  T operator()(const TIn& neighbors) const
  {
    return std::accumulate(neighbors.begin(), neighbors.end(), T()) / neighbors.size();
  }
};

/**
 * @brief Median filtering.
 */
template <typename T>
struct MedianFilter {
  using Value = T;

  template <typename TIn>
  T operator()(const TIn& neighbors) const
  {
    std::vector<Value> v(neighbors.begin(), neighbors.end());
    const auto size = v.size();
    auto b = v.data();
    auto e = b + size;
    auto n = b + size / 2;
    std::nth_element(b, n, e);
    if (size % 2 == 1) {
      return *n;
    }
    std::nth_element(b, n + 1, e);
    return (*n + *(n + 1)) * .5;
  }
};

/**
 * @brief Erosion (i.e. min filtering).
 */
template <typename T>
struct Erosion {
  using Value = T;

  template <typename TIn>
  T operator()(const TIn& neighbors) const
  {
    return *std::min_element(neighbors.begin(), neighbors.end());
  }
};

/**
 * @brief Dilation (i.e. max filtering).
 */
template <typename T>
struct Dilation {
  using Value = T;

  template <typename TIn>
  T operator()(const TIn& neighbors) const
  {
    return *std::max_element(neighbors.begin(), neighbors.end());
  }
};

} // namespace MorphologyOp

/**
 * @brief A structuring element for morphological operations.
 * @tparam TOp The morphological operator
 * @tparam T The value type
 * @tparam TWindow The type of window, e.g. `Box` or `Mask`
 */
template <typename TOp, typename TWindow>
class StructuringElement : public FilterMixin<typename TOp::Value, TWindow, StructuringElement<TOp, TWindow>> {
public:

  /**
   * @brief The value type.
   */
  using Value = typename TOp::Value;

  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = TWindow::Dimension;

  /**
   * @brief Explcit window constructor.
   * @param window The filter window
   */
  explicit StructuringElement(TOp&& op, TWindow window) :
      FilterMixin<Value, TWindow, StructuringElement<TOp, TWindow>>(std::move(window)), m_op(std::forward<TOp>(op))
  {}

  /**
   * @brief Hypercube window constructor.
   * @param radius The hypercube radius
   */
  explicit StructuringElement(TOp&& op, Index radius = 1) :
      StructuringElement(std::forward<TOp>(op), Box<Dimension>::from_center(radius))
  {}

  /**
   * @brief Estimation operator.
   */
  template <typename TIn>
  inline Value operator()(const TIn& neighbors) const
  {
    return m_op(neighbors);
  }

private:

  /**
   * @brief The operation.
   */
  TOp m_op;
};

template <typename T, typename TWindow>
StructuringElement<MorphologyOp::MeanFilter<T>, TWindow> mean_filter(TWindow window)
{
  return StructuringElement<MorphologyOp::MeanFilter<T>, TWindow>(MorphologyOp::MeanFilter<T> {}, std::move(window));
}

template <typename T, typename TWindow>
StructuringElement<MorphologyOp::MedianFilter<T>, TWindow> median_filter(TWindow window)
{
  return StructuringElement<MorphologyOp::MedianFilter<T>, TWindow>(
      MorphologyOp::MedianFilter<T> {},
      std::move(window));
}

template <typename T, typename TWindow>
StructuringElement<MorphologyOp::Erosion<T>, TWindow> erosion(TWindow window)
{
  return StructuringElement<MorphologyOp::Erosion<T>, TWindow>(MorphologyOp::Erosion<T> {}, std::move(window));
}

template <typename T, typename TWindow>
StructuringElement<MorphologyOp::Dilation<T>, TWindow> dilation(TWindow window)
{
  return StructuringElement<MorphologyOp::Dilation<T>, TWindow>(MorphologyOp::Dilation<T> {}, std::move(window));
}

} // namespace Linx

#endif
