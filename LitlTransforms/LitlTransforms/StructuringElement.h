// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_STRUCTURINGELEMENT_H
#define _LITLTRANSFORMS_STRUCTURINGELEMENT_H

#include "LitlRaster/Raster.h"
#include "LitlTransforms/Interpolation.h"

namespace Litl {

/**
   * @brief Compute the sum of the neighbors.
   */
template <typename T>
T sum(const std::vector<std::decay_t<T>>& neighbors) {
  return std::accumulate(neighbors.begin(), neighbors.end(), T());
}

/**
   * @brief Compute the mean of the neighbors.
   */
template <typename T>
T mean(const std::vector<std::decay_t<T>>& neighbors) {
  return sum(neighbors) / neighbors.size();
}

/**
   * @brief Get the n-th neighbor value.
   */
template <typename T>
T nth(std::vector<std::decay_t<T>>& neighbors, std::size_t n) {
  auto b = neighbors.data();
  auto e = b + neighbors.size();
  auto out = b + n;
  std::nth_element(b, out, e);
  return *out;
}

/**
   * @brief Compute the median of the neighbors.
   */
template <typename T>
T median(std::vector<std::decay_t<T>>& neighbors) {
  const auto size = neighbors.size();
  auto b = neighbors.data();
  auto e = b + size;
  auto n = b + size / 2;
  std::nth_element(b, n, e);
  if (size % 2 == 1) {
    return *n;
  }
  std::nth_element(b, n + 1, e);
  return (*n + *(n + 1)) * .5;
}

/**
   * @brief Get the min neighbor value.
   */
template <typename T>
T min(const std::vector<std::decay_t<T>>& neighbors) {
  return *std::min_element(neighbors.begin(), neighbors.end());
}

/**
   * @brief Get the max neighbor value.
   */
template <typename T>
T max(const std::vector<std::decay_t<T>>& neighbors) {
  return *std::max_element(neighbors.begin(), neighbors.end());
}

/**
 * @brief A structuring element for morphological operations.
 * @tparam TWindow The type of window, e.g. `Box` or `Mask`
 */
template <typename TWindow>
class StructuringElement {

public:
  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = TWindow::Dimension;

  /**
   * @brief Explcit window constructor.
   * @param window The filter window
   */
  explicit StructuringElement(TWindow window) : m_window(std::move(window)) {}

  /**
   * @brief Hypercube window constructor.
   * @param radius The hypercube radius
   */
  explicit StructuringElement(Index radius = 1) : StructuringElement(Box<Dimension>::fromCenter(radius)) {}

  /**
   * @brief Apply a median filter.
   * @see `apply()`
   */
  template <typename TIn>
  Raster<std::decay_t<typename TIn::Value>, TIn::Dimension> median(const TIn& in) {
    return apply(&Litl::median<typename TIn::Value>, in);
  }

  /**
   * @brief Apply a median filter into a given output raster.
   * @see `applyTo()`
   */
  template <typename TIn, typename TOut>
  void medianTo(const TIn& in, TOut& out, const Box<Dimension>& region = Box<Dimension>::whole()) {
    return applyTo(&Litl::median<typename TIn::Value>, in, out, region);
  }

  /**
   * @brief Erode.
   * @see `apply()`
   */
  template <typename TIn>
  Raster<std::decay_t<typename TIn::Value>, TIn::Dimension> erode(const TIn& in) {
    return apply(&Litl::min<typename TIn::Value>, in);
  }

  /**
   * @brief Erode into a given output raster.
   * @see `applyTo()`
   */
  template <typename TIn, typename TOut>
  void erodeTo(const TIn& in, TOut& out, const Box<Dimension>& region = Box<Dimension>::whole()) {
    return applyTo(&Litl::min<typename TIn::Value>, in, out, region);
  }

  /**
   * @brief Dilate.
   * @see `apply()`
   */
  template <typename TIn>
  Raster<std::decay_t<typename TIn::Value>, TIn::Dimension> dilate(const TIn& in) {
    return apply(&Litl::max<typename TIn::Value>, in);
  }

  /**
   * @brief Dilate into a given output raster.
   * @see `applyTo()`
   */
  template <typename TIn, typename TOut>
  void dilateTo(const TIn& in, TOut& out, const Box<Dimension>& region = Box<Dimension>::whole()) {
    return applyTo(&Litl::max<typename TIn::Value>, in, out, region);
  }

  /**
   * @brief Apply a function.
   * @see `applyTo()`
   */
  template <typename TFunc, typename TIn>
  Raster<std::decay_t<typename TIn::Value>, TIn::Dimension> apply(TFunc&& func, const TIn& in) {
    Raster<std::decay_t<typename TIn::Value>, TIn::Dimension> out(in.shape());
    applyTo(std::forward<TFunc>(func), in, out, out.domain());
    return out;
  }

  /**
   * @brief Apply to each pixel of a region a function which transforms neighbors into a value.
   * @param func The function to apply
   * @param in The input raster or extrapolator
   * @param out The output raster
   * @param region The application region
   * @details
   * For each pixel of `in` inside `region`, get the neighboring values, apply the function to them,
   * and assign the output of the function to the corresponding pixel in `out`.
   */
  template <typename TFunc, typename TIn, typename TOut>
  void applyTo(TFunc&& func, const TIn& in, TOut& out, const Box<Dimension>& region = Box<Dimension>::whole()) {

    std::vector<std::decay_t<typename TOut::Value>> neighbors(m_window.size());

    // Non-extrapolated pixels
    auto inner = in.domain() - box(m_window);
    // inner.clamp(region); // FIXME
    applyRegionTo(std::forward<TFunc>(func), rasterize(in), inner, out);

    // Extrapolated pixels
    auto outers = inner.surround(box(m_window));
    for (auto& o : outers) {
      // o.clamp(region); // FIXME
      applyRegionTo(std::forward<TFunc>(func), in, o, out);
    }
  }

  template <typename TFunc, typename TIn, typename TOut>
  void applyRegionTo(TFunc&& func, const TIn& in, const Box<TIn::Dimension>& region, TOut& out) {
    if (region.size() < 0) {
      return;
    }
    auto patch = in.subraster(m_window);
    std::vector<std::decay_t<typename TIn::Value>> neighbors(m_window.size());
    for (const auto& p : region) {
      patch.shift(p);
      std::copy(patch.begin(), patch.end(), neighbors.begin());
      out[p] = std::forward<TFunc>(func)(neighbors);
      // FIXME replace out[p] with an iterator
      patch.shiftBack(p);
    }
  }

private:
  /**
   * @brief The window.
   */
  TWindow m_window;
};

} // namespace Litl

#endif
