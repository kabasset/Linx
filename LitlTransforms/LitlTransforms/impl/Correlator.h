// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_IMPL_CORRELATOR_H
#define _LITLTRANSFORMS_IMPL_CORRELATOR_H

#include "LitlCore/BorderedBox.h"
#include "LitlCore/Box.h"
#include "LitlCore/Grid.h"
#include "LitlCore/Raster.h"
#include "LitlTransforms/Extrapolation.h"

namespace Litl {

/**
 * @brief Mixin to implement cross-correlation strategies.
 */
template <typename T, Index N, typename TDerived>
class CorrelatorMixin {

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
   * @brief Correlate an input extrapolator.
   * @see `correlateTo()`
   */
  template <typename TExtrapolator>
  Raster<Value, Dimension> operator*(const TExtrapolator& in) const {
    Raster<Value, Dimension> out(rasterize(in).shape());
    out.fill(Value());
    correlateTo(in, out);
    return out;
  }

  /**
   * @brief Correlate an input raster extrapolator or patch into an output raster.
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
  void correlateTo(const TIn& in, TOut& out) const {
    if constexpr (isExtrapolator<TIn>()) {
      correlateSplitTo(in, out);
    } else {
      // FIXME check no extrapolation is required
      static_cast<const TDerived&>(*this).correlateMonolithTo(in, out);
    }
  }

  /**
   * @brief Correlate and crop an input extrapolator.
   * 
   * The input patch can be a box for a cropped correlation,
   * or a grid for a decimated correlation.
   * 
   * @see `correlateCropTo()`
   */
  template <typename TPatch>
  Raster<Value, Dimension> correlateCrop(const TPatch& in) const {
    // FIXME check region is a Box or Grid
    Raster<Value, Dimension> out(in.domain().shape());
    correlateTo(in, out);
    return out;
  }

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
  template <typename TIn, typename TRaster>
  void correlateSplitTo(const TIn& in, TRaster& out) const {
    const auto& raw = dontExtrapolate(in);
    const auto box = BorderedBox<Dimension>(Litl::box(raw.domain()), static_cast<const TDerived&>(*this).window());
    box.applyInnerBorder(
        [&](const auto& ib) {
          const auto insub = raw.patch(ib);
          auto outsub = out.patch(insub.domain());
          static_cast<const TDerived&>(*this).correlateMonolithTo(insub, outsub);
        },
        [&](const auto& ib) {
          const auto insub = in.patch(ib);
          auto outsub = out.patch(insub.domain());
          static_cast<const TDerived&>(*this).correlateMonolithTo(insub, outsub);
        });
  }
};

} // namespace Litl

#endif
