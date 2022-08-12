// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_IMPL_CORRELATOR_H
#define _LITLTRANSFORMS_IMPL_CORRELATOR_H

#include "LitlRaster/BorderedBox.h"
#include "LitlRaster/Box.h"
#include "LitlRaster/Grid.h"
#include "LitlRaster/Raster.h"
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
    correlateTo(in, out);
    return out;
  }

  /**
   * @brief Correlate an input (raster or subraster) extrapolator into an output raster.
   * @param in A extrapolator or subraster or subextrapolator
   * @param out A raster
   * 
   * If the correlation bounding box requires input values from outside the input domain,
   * then `in` must be an extrapolator or subextrapolator.
   * If the bounding box of `in` is small enough so than no extrapolated values are required,
   * then `in` can be a subraster.
   * 
   * @see `correlateCropTo()`
   */
  template <typename TIn, typename TRaster>
  void correlateTo(const TIn& in, TRaster& out) const {
    const auto& notExtrapolated = dontExtrapolate(in); // A raster or subraster
    const auto box = BorderedBox<Dimension>(rasterize(in).domain(), static_cast<const TDerived&>(*this).window());
    box.applyInnerBorder(
        [&](const auto& ib) {
          std::cout << "Inner: " << ib.front() << " - " << ib.back() << std::endl;
          const auto insub = notExtrapolated.subraster(ib);
          auto outsub = out.subraster(insub.domain());
          static_cast<const TDerived&>(*this).correlateMonolithTo(insub, outsub);
        },
        [&](const auto& ib) {
          std::cout << "Border: " << ib.front() << " - " << ib.back() << std::endl;
          const auto insub = in.subraster(ib);
          auto outsub = out.subraster(insub.domain());
          static_cast<const TDerived&>(*this).correlateMonolithTo(insub, outsub);
        });
  }

  /**
   * @brief Correlate and crop an input extrapolator.
   * @see `correlateCropTo()`
   */
  template <typename TBoxExtrapolator>
  Raster<Value, Dimension> correlateCrop(const TBoxExtrapolator& in) const {
    Raster<Value, Dimension> out(in.domain().shape());
    correlateCropTo(in, out);
    return out;
  }

  /**
   * @brief Correlate and crop an input extrapolator into an output raster.
   * @param in A subextrapolator with box region
   * @param out A raster
   */
  template <typename TBoxExtrapolator, typename TRaster>
  void correlateCropTo(const TBoxExtrapolator& in, TRaster& out) const {
    const auto& inner = dontExtrapolate(in);
    const auto& front = in.domain().front();
    const auto box = BorderedBox<Dimension>(rasterize(in).domain(), static_cast<const TDerived&>(*this).window());
    box.applyInnerBorder(
        [&](const auto& ib) {
          auto outsub = out.subraster(ib - front);
          static_cast<const TDerived&>(*this).correlateMonolithTo(inner.subraster(ib), outsub);
        },
        [&](const auto& ib) {
          auto outsub = out.subraster(ib - front);
          static_cast<const TDerived&>(*this).correlateMonolithTo(in.subraster(ib), outsub);
        });
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
    const auto& inner = dontExtrapolate(in);
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
          const auto insub = inner.subraster(ib);
          auto outsub = out.subraster(gridToBox(insub.domain()));
          static_cast<const TDerived&>(*this).correlateMonolithTo(insub, outsub);
        },
        [&](const auto& ib) {
          const auto insub = in.subraster(ib);
          auto outsub = out.subraster(gridToBox(insub.domain()));
          static_cast<const TDerived&>(*this).correlateMonolithTo(insub, outsub);
        });
  }
};

} // namespace Litl

#endif