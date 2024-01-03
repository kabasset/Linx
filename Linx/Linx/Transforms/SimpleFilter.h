// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_SIMPLEFILTER_H
#define _LINXTRANSFORMS_SIMPLEFILTER_H

#include "Linx/Data/Raster.h"
#include "Linx/Transforms/Extrapolation.h"
#include "Linx/Transforms/mixins/Filter.h"

namespace Linx {

/**
 * @ingroup filtering
 * @brief Structuring element-based operations.
 */
namespace MorphologyOp { // FIXME rm

/**
 * @ingroup filtering
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
 * @ingroup filtering
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
 * @ingroup filtering
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
 * @ingroup filtering
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
 * @ingroup filtering
 * @brief A structuring element for morphological operations.
 * @tparam TOp The morphological operator
 * @tparam T The value type
 * @tparam TWindow The type of window, e.g. `Box` or `Mask`
 */
template <typename TOp, typename TWindow>
class SimpleFilter : public FilterMixin<typename TOp::Value, TWindow, SimpleFilter<TOp, TWindow>> {
  friend class FilterMixin<typename TOp::Value, TWindow, SimpleFilter<TOp, TWindow>>; // FIXME simplify

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
  explicit SimpleFilter(TOp&& op, TWindow window) : m_op(std::forward<TOp>(op)), m_window(std::forward<TWindow>(window))
  {}

  /**
   * @brief Hypercube window constructor.
   * @param radius The hypercube radius
   */
  explicit SimpleFilter(TOp&& op, Index radius = 1) :
      SimpleFilter(std::forward<TOp>(op), Box<Dimension>::from_center(radius))
  {}

protected:

  /**
   * @brief Get the filtering window.
   */
  const TWindow& window_impl() const
  {
    return m_window;
  }

  /**
   * @brief Filter and crop an input raster.
   * 
   * Border regions which would require extrapolation are cropped-out,
   * such that the output domain is `in.domain() - filter.window()`.
   */
  template <typename T, Index N, typename THolder, typename TOut>
  void transform_impl(const Raster<T, N, THolder>& in, TOut& out) const
  {
    const auto region = in.domain() - Linx::box(window_impl());
    transform_monolith(in(region), out);
  }

  /**
   * @brief Filter an extrapolated raster.
   * 
   * The output raster has the same shape as the input raster.
   */
  template <typename TRaster, typename TMethod, typename TOut>
  void transform_impl(const Extrapolation<TRaster, TMethod>& in, TOut& out) const
  {
    const auto& raw = dont_extrapolate(in);
    const auto bbox =
        Internal::BorderedBox<TRaster::Dimension>(raw.domain(), extend<TRaster::Dimension>(box(m_window)));
    bbox.apply_inner_border(
        [&](const auto& ib) {
          const auto insub = raw(ib);
          if (insub.size() > 0) {
            auto outsub = out(insub.domain());
            transform_monolith(insub, outsub);
          }
        },
        [&](const auto& ib) {
          const auto insub = in(ib);
          if (insub.size() > 0) {
            auto outsub = out(insub.domain());
            transform_monolith_extrapolator(insub, outsub);
          }
        });
  }

  /**
   * @brief Filter and decimate a grid-based patch.
   * 
   * Decimation is especially useful for downsampling.
   * 
   * If input values from outside the raster domain are required,
   * then `in` must be an extrapolator.
   * On the contrary, if the box of `in` is small enough so that no extrapolated values are required,
   * then `in` can be a raw patch.
   * 
   * The output raster has the same shape as the grid.
   */
  template <typename T, typename TParent, typename TRegion, typename TOut>
  void transform_impl(const Patch<T, TParent, TRegion>& in, TOut& out) const
  {
    const auto& raw = dont_extrapolate(in);
    const auto& front = in.domain().front();
    const auto& step = in.domain().step();

    const auto grid_to_box = [&](const auto& g) {
      auto f = g.front() - front;
      for (std::size_t i = 0; i < f.size(); ++i) { // FIXME simplify with Linx
        f[i] /= step[i];
      }
      return Box<TParent::Dimension>::from_shape(f, g.shape());
    };

    const auto window = extend<TParent::Dimension>(box(m_window));
    decltype(auto) domain = rasterize(in).domain();
    const auto bbox = Internal::BorderedBox<TParent::Dimension>(domain, window);
    // FIXME accept non-Box window, and of lower dim
    bbox.apply_inner_border(
        [&](const auto& ib) {
          const auto insub = raw(ib);
          if (insub.size() > 0) { // FIXME needed?
            auto outsub = out(grid_to_box(insub.domain()));
            transform_monolith(insub, outsub);
          }
        },
        [&](const auto& ib) {
          const auto insub = in(ib);
          if (insub.size() > 0) {
            auto outsub = out(grid_to_box(insub.domain()));
            transform_monolith(insub, outsub);
          }
        });
  }

private:

  /**
   * @brief Filter a monolithic patch (no region splitting).
   */
  template <typename TIn, typename TOut>
  void transform_monolith_extrapolator(const TIn& in, TOut& out) const
  {
    const auto extrapolated = in.parent().copy(Linx::box(in.domain()) + window_impl());
    const auto box = extrapolated.domain() - window_impl();
    // FIXME region - window().front()?
    transform_monolith(extrapolated(box), out);
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
  void transform_monolith(const TIn& in, TOut& out) const
  {
    // FIXME accept any region
    auto patch = in.parent()(extend<TIn::Dimension>(m_window));
    auto out_it = out.begin();
    for (const auto& p : in.domain()) {
      patch >>= p;
      *out_it = m_op(patch);
      ++out_it;
      patch <<= p;
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

/**
 * @ingroup filtering
 */
template <typename T, typename TWindow>
SimpleFilter<MorphologyOp::MeanFilter<T>, TWindow> mean_filter(TWindow window)
{
  return SimpleFilter<MorphologyOp::MeanFilter<T>, TWindow>(MorphologyOp::MeanFilter<T> {}, std::move(window));
}

/**
 * @ingroup filtering
 */
template <typename T, typename TWindow>
SimpleFilter<MorphologyOp::MedianFilter<T>, TWindow> median_filter(TWindow window)
{
  return SimpleFilter<MorphologyOp::MedianFilter<T>, TWindow>(MorphologyOp::MedianFilter<T> {}, std::move(window));
}

/**
 * @ingroup filtering
 */
template <typename T, typename TWindow>
SimpleFilter<MorphologyOp::Erosion<T>, TWindow> erosion(TWindow window)
{
  return SimpleFilter<MorphologyOp::Erosion<T>, TWindow>(MorphologyOp::Erosion<T> {}, std::move(window));
}

/**
 * @ingroup filtering
 */
template <typename T, typename TWindow>
SimpleFilter<MorphologyOp::Dilation<T>, TWindow> dilation(TWindow window)
{
  return SimpleFilter<MorphologyOp::Dilation<T>, TWindow>(MorphologyOp::Dilation<T> {}, std::move(window));
}

} // namespace Linx

#endif
