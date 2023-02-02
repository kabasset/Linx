// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_IMPL_FILTER_H
#define _LITLTRANSFORMS_IMPL_FILTER_H

#include "LitlCore/BorderedBox.h"
#include "LitlCore/Box.h"
#include "LitlCore/Grid.h"
#include "LitlCore/Raster.h"
#include "LitlTransforms/Extrapolation.h"

namespace Litl {

/**
 * @brief Spatial filtering mixin.
 */
template <typename TKernel, typename TFunc>
class Filter {

public:
  /**
   * @brief The value type.
   */
  using Value = typename TKernel::Value;

  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = TKernel::Dimension;

  /**
   * @brief Constructor.
   */
  Filter(const Box<Dimension>& window, TFunc&& func) : m_window(window), m_func(std::forward<TFunc>(func)) {}

  /**
   * @brief Filter an input extrapolator or patch.
   * @see `to()`
   */
  template <typename TIn>
  Raster<Value, Dimension> full(const TIn& in) const {
    Raster<Value, Dimension> out(rasterize(in).shape());
    out.fill(Value());
    transform(in, out);
    return out;
  }

  /**
   * @brief Correlate and crop an input extrapolator.
   * @see `correlateCropTo()`
   */
  template <typename TPatch>
  Raster<Value, Dimension> crop(const TPatch& in) const {
    // FIXME check region is a Box or Grid
    Raster<Value, Dimension> out(in.domain().shape());
    transform(in, out);
    return out;
  }

  /**
   * @brief Filter an input raster extrapolator or patch into an output raster.
   * @param in A extrapolator or patch or subextrapolator
   * @param out A raster or patch with compatible domain
   * 
   * If the correlation bounding box requires input values from outside the input domain,
   * then `in` must be an extrapolator.
   * If the bounding box of `in` is small enough so that no extrapolated values are required,
   * then `in` can be a patch.
   * 
   * @see `correlateCropTo()`
   */
  template <typename TIn, typename TOut>
  void transform(const TIn& in, TOut& out) const {
    if constexpr (isExtrapolator<TIn>()) {
      transformSplits(in, out);
    } else {
      // FIXME check no extrapolation is required
      transformMonolith(in, out);
    }
  }

private:
  /**
   * @brief Correlate an input (raster or patch) extrapolator into an output raster.
   * @param in A extrapolator or patch or subextrapolator
   * @param out A raster
   * 
   * If the correlation bounding box requires input values from outside the input domain,
   * then `in` must be an extrapolator or subextrapolator.
   * If the bounding box of `in` is small enough so than no extrapolated values are required,
   * then `in` can be a patch.
   * 
   * @see `correlateCropTo()`
   */
  template <typename TIn, typename TOut>
  void transformSplits(const TIn& in, TOut& out) const {
    const auto& raw = dontExtrapolate(in);
    const auto box = BorderedBox<Dimension>(Litl::box(raw.domain()), m_window);
    box.applyInnerBorder(
        [&](const auto& ib) {
          const auto insub = raw.patch(ib);
          auto outsub = out.patch(insub.domain());
          transformMonolith(insub, outsub);
        },
        [&](const auto& ib) {
          const auto insub = in.patch(ib);
          auto outsub = out.patch(insub.domain());
          transformMonolith(insub, outsub);
        });
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
  void transformMonolith(const TIn& in, TOut& out) const {
    auto patch = in.parent().patch(m_window);
    auto outIt = out.begin();
    for (const auto& p : in.domain()) {
      patch.translate(p);
      *outIt = m_func(patch); // FIXME forward?
      ++outIt;
      patch.translateBack(p);
    }
  }

private:
  /**
   * @brief The filtering window.
   */
  const Box<Dimension>& m_window; // FIXME allow StructuringElement "kernels"

  /**
   * @brief The filtering function.
   */
  TFunc m_func;
};

/**
 * @brief Make a filter.
 */
template <typename TKernel, typename TFunc>
Filter<TKernel, typename std::decay_t<TFunc>> filterize(const TKernel& kernel, TFunc&& func) {
  return Filter<TKernel, typename std::decay_t<TFunc>>(kernel.window(), std::forward<TFunc>(func));
}

} // namespace Litl

#endif
