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
T sum(const std::vector<T>& neighbors) {
  return std::accumulate(neighbors.begin(), neighbors.end(), T());
}

/**
   * @brief Compute the mean of the neighbors.
   */
template <typename T>
T mean(const std::vector<T>& neighbors) {
  return sum(neighbors) / neighbors.size();
}

/**
   * @brief Get the n-th neighbor value.
   */
template <typename T>
T nth(std::vector<T>& neighbors, std::size_t n) {
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
T median(std::vector<T>& neighbors) {
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
T min(const std::vector<T>& neighbors) {
  return *std::min_element(neighbors.begin(), neighbors.end());
}

/**
   * @brief Get the max neighbor value.
   */
template <typename T>
T max(const std::vector<T>& neighbors) {
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
  StructuringElement(const TWindow& window) : m_window(window), m_offsets(m_window.size()) {}

  /**
   * @brief Hypercube window constructor.
   * @param radius The cube radius
   */
  StructuringElement(Index radius = 1) : StructuringElement(Box<Dimension>::fromCenter(radius)) {}

  /**
   * @brief Apply a median filter.
   * @see `apply()`
   */
  template <typename TIn>
  Raster<typename TIn::Value, TIn::Dimension> median(const TIn& in) {
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
   * @brief Erode into a given output raster.
   * @see `applyTo()`
   */
  template <typename TIn, typename TOut>
  void erodeTo(const TIn& in, TOut& out, const Box<Dimension>& region = Box<Dimension>::whole()) {
    return applyTo(&Litl::min<typename TIn::Value>, in, out, region);
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
  Raster<typename TIn::Value, TIn::Dimension> apply(TFunc&& func, const TIn& in) {
    Raster<typename TIn::Value, TIn::Dimension> out(in.shape());
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

    std::vector<typename TOut::Value> neighbors(m_window.size());

    // Non-extrapolated pixels
    auto inner = in.domain() - box(m_window);
    inner.clamp(region);
    computeOffsets(rasterize(in));
    auto innerSub = out.subraster(inner);
    auto innerIt = begin(innerSub);
    for (const auto& p : inner) {
      loadNeighborsWithoutExtrapolation(rasterize(in), p, neighbors);
      *innerIt++ = std::forward<TFunc>(func)(neighbors);
    }

    // Extrapolated pixels
    auto outers = inner.surround(box(m_window));
    for (auto& o : outers) {
      o.clamp(region);
      auto oSub = out.subraster(o);
      auto oIt = begin(oSub);
      for (const auto& p : o) {
        loadNeighborsWithExtrapolation(in, p, neighbors);
        *oIt++ = std::forward<TFunc>(func)(neighbors);
      }
    }
  }

private:
  /**
   * @brief Compute the index offsets of non-extrapolated neighbors.
   */
  template <typename TRaster>
  void computeOffsets(const TRaster& in) {
    std::transform(m_window.begin(), m_window.end(), m_offsets.begin(), [&](const auto& p) {
      return in.index(p);
    });
  }

  /**
   * @brief Load the values of the non-extrapolated neighbors.
   */
  template <typename TRaster, typename TOut>
  void loadNeighborsWithoutExtrapolation(const TRaster& in, const Position<Dimension>& p, TOut& neighbors) {
    const auto* center = &in[p];
    std::transform(m_offsets.begin(), m_offsets.end(), neighbors.begin(), [&](auto o) {
      return *(center + o);
    });
  }

  /**
   * @brief Load the values of the possibly extrapolated neighbors.
   */
  template <typename TExtrapolator, typename TOut>
  void loadNeighborsWithExtrapolation(const TExtrapolator& in, const Position<Dimension>& p, TOut& neighbors) {
    auto it = neighbors.begin();
    for (const auto& q : m_window) {
      *it++ = in[p + q]; // FIXME move the window to limit positions additions? specialize?
    }
  }

private:
  /**
   * @brief The window.
   */
  TWindow m_window;

  /**
   * @brief The index offsets of the neighboring pixels.
   */
  std::vector<Index> m_offsets;
};

} // namespace Litl

#endif
