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
#include "LitlTransforms/Interpolation.h"

#include <iostream> // std::cout

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
      if constexpr (isPatch<TIn>()) {
        correlatePatchTo(in, out);
      } else {
        correlateRasterTo(in, out);
      }
    } else {
      // FIXME check no extrapolation is required
      static_cast<const TDerived&>(*this).correlateMonolithTo(in, out);
    }
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
  template <typename TIn, typename Tout>
  void correlateRasterTo(const TIn& in, Tout& out) const {
    const auto& raster = dontExtrapolate(in);
    const auto box = BorderedBox<Dimension>(in.domain(), static_cast<const TDerived&>(*this).window());
    box.applyInnerBorder(
        [&](const auto& ib) {
          const auto insub = raster.patch(ib);
          auto outsub = out.patch(insub.domain());
          static_cast<const TDerived&>(*this).correlateMonolithTo(insub, outsub);
        },
        [&](const auto& ib) {
          const auto insub = in.patch(ib);
          auto outsub = out.patch(insub.domain());
          static_cast<const TDerived&>(*this).correlateMonolithTo(insub, outsub);
        });
  }

  template <typename TIn, typename TRaster>
  void correlatePatchTo(const TIn& in, TRaster& out) const {
    const auto& patch = dontExtrapolate(in);
    const auto box = BorderedBox<Dimension>(Litl::box(patch.domain()), static_cast<const TDerived&>(*this).window());
    box.applyInnerBorder(
        [&](const auto& ib) {
          std::cout << "Patch inner: " << ib.front() << " - " << ib.back() << std::endl;
          const auto insub = patch.patch(ib);
          auto outsub = out.patch(insub.domain());
          static_cast<const TDerived&>(*this).correlateMonolithTo(insub, outsub);
        },
        [&](const auto& ib) {
          std::cout << "Patch border: " << ib.front() << " - " << ib.back() << std::endl;
          const auto insub = in.patch(ib);
          auto outsub = out.patch(insub.domain());
          static_cast<const TDerived&>(*this).correlateMonolithTo(insub, outsub);
        });
  }

  /**
   * @brief Correlate and crop an input extrapolator.
   * @see `correlateCropTo()`
   */
  template <typename TPatch>
  Raster<Value, Dimension> correlateCrop(const TPatch& in) const {
    // FIXME check region is a Box
    Raster<Value, Dimension> out(in.domain().shape());
    correlateTo(in, out);
    return out;
  }

  /**
   * @brief Correlate and decimate an input extrapolator.
   * @see `correlateDecimateTo()`
   */
  template <typename TGridExtrapolator>
  Raster<Value, Dimension> correlateDecimate(const TGridExtrapolator& in) const {
    Raster<Value, Dimension> out(in.domain().shape());
    correlateDecimateTo(in, out);
    return out;
  }

  /**
   * @brief Correlate and decimate an input extrapolator into an output raster.
   * @param in A subextrapolator with grid region
   * @param out A raster
   */
  template <typename TGridExtrapolator, typename TRaster>
  void correlateDecimateTo(const TGridExtrapolator& in, TRaster& out) const {
    const auto& notExtrapolated = dontExtrapolate(in);
    const auto& grid = in.domain();
    const auto& front = grid.front();
    const auto& step = grid.step();
    const auto gridToBox = [&](const auto& g) {
      auto f = g.front() - front;
      for (std::size_t i = 0; i < f.size(); ++i) {
        f /= step[i];
      }
      return Box<Dimension>::fromShape(f, g.shape());
    };
    const auto box = BorderedBox<Dimension>(rasterize(in).domain(), static_cast<const TDerived&>(*this).window());
    box.applyInnerBorder(
        [&](const auto& ib) {
          const auto insub = notExtrapolated.patch(ib);
          auto outsub = out.patch(gridToBox(insub.domain()));
          static_cast<const TDerived&>(*this).correlateMonolithTo(insub, outsub);
        },
        [&](const auto& ib) {
          const auto insub = in.patch(ib);
          auto outsub = out.patch(gridToBox(insub.domain()));
          static_cast<const TDerived&>(*this).correlateMonolithTo(insub, outsub);
        });
  }
};

} // namespace Litl

#endif
