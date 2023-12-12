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
class Filter : public FilterMixin<typename TOp::Value, TWindow, Filter<TOp, TWindow>> {
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
  explicit Filter(TOp&& op, TWindow window) : m_op(std::forward<TOp>(op)), m_window(std::forward<TWindow>(window)) {}

  /**
   * @brief Hypercube window constructor.
   * @param radius The hypercube radius
   */
  explicit Filter(TOp&& op, Index radius = 1) : Filter(std::forward<TOp>(op), Box<Dimension>::from_center(radius)) {}

  /**
   * @brief Get the filtering window.
   */
  const TWindow& window() const
  {
    return m_window;
  }

  /**
   * @brief Filter an input extrapolator or patch into an output raster or patch.
   * @param in A extrapolator or patch (or both)
   * @param out A raster or patch with compatible domain
   * 
   * If the filtering bounding box requires input values from outside the input domain,
   * then `in` must be an extrapolator.
   * If the bounding box of `in` is small enough so that no extrapolated values are required,
   * then `in` can be a raw patch.
   */
  template <typename TIn, typename TOut>
  void transform(const TIn& in, TOut& out) const
  {
    // FIXME accept any region
    auto patch = in.parent().patch(extend<TIn::Dimension>(m_window));
    auto out_it = out.begin();
    for (const auto& p : in.domain()) {
      patch.translate(p);
      *out_it = m_op(patch);
      ++out_it;
      patch.translate_back(p);
    }
  }

private:

  /**
   * @brief The operation.
   */
  TOp m_op;

  /**
   * @brief The window with origin at position 0.
   */
  TWindow m_window;
};

template <typename T, typename TWindow>
Filter<MorphologyOp::MeanFilter<T>, TWindow> mean_filter(TWindow window)
{
  return Filter<MorphologyOp::MeanFilter<T>, TWindow>(MorphologyOp::MeanFilter<T> {}, std::move(window));
}

template <typename T, typename TWindow>
Filter<MorphologyOp::MedianFilter<T>, TWindow> median_filter(TWindow window)
{
  return Filter<MorphologyOp::MedianFilter<T>, TWindow>(MorphologyOp::MedianFilter<T> {}, std::move(window));
}

template <typename T, typename TWindow>
Filter<MorphologyOp::Erosion<T>, TWindow> erosion(TWindow window)
{
  return Filter<MorphologyOp::Erosion<T>, TWindow>(MorphologyOp::Erosion<T> {}, std::move(window));
}

template <typename T, typename TWindow>
Filter<MorphologyOp::Dilation<T>, TWindow> dilation(TWindow window)
{
  return Filter<MorphologyOp::Dilation<T>, TWindow>(MorphologyOp::Dilation<T> {}, std::move(window));
}

} // namespace Linx

#endif
