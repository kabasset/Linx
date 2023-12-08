// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_IMPL_FILTER_H
#define _LINXTRANSFORMS_IMPL_FILTER_H

#include "Linx/Data/BorderedBox.h"
#include "Linx/Data/Box.h"
#include "Linx/Data/Grid.h"
#include "Linx/Data/Raster.h"
#include "Linx/Transforms/Extrapolation.h"

namespace Linx {

/**
 * @brief Spatial filtering mixin.
 */
template <typename T, typename TWindow, typename TDerived>
class FilterMixin {
public:

  /**
   * @brief The value type.
   */
  using Value = T;

  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = TWindow::Dimension;

  /**
   * @brief Constructor.
   */
  FilterMixin(TWindow window) : m_window(std::move(window)) {}

  /**
   * @brief Get the filtering window.
   */
  const TWindow& window() const
  {
    return m_window;
  }

  /**
   * @brief Apply the filter according to the input type.
   * 
   * Here is the mapping between the input type and associated operation:
   * - If `in` is an extrapolated raster, then `full(in)` is returned;
   * - If `in` is a raw raster, then `crop(in)` is returned;
   * - If `in` is a box-based patch, then `box(in)` is returned;
   * - If `in` is a grid-based batch, then `decimate(in)` is returned.
   * 
   * For any other case, the method won't compile (see `transform()`).
   * 
   */
  template <typename TIn>
  Raster<Value, TIn::Dimension> operator*(const TIn& in) const
  {
    // FIXME support arbitrary patches
    if constexpr (is_patch<TIn>()) {
      if (in.domain().step().is_one()) { // Box
        return box(in);
      } else { // Grid
        return decimate(in);
      }
    } else if constexpr (is_extrapolator<TIn>()) {
      return full(in);
    } else {
      return crop(in);
    }
  }

  /**
   * @brief Filter an extrapolated raster.
   * 
   * The output raster has the same shape as the input raster.
   */
  template <typename TExtrapolator>
  Raster<Value, TExtrapolator::Dimension> full(const TExtrapolator& in) const
  {
    Raster<Value, TExtrapolator::Dimension> out(in.raster().shape());
    transform_splits(in, out);
    return out;
  }

  /**
   * @brief Filter and crop an input raster.
   * 
   * Border regions which would require extrapolation are cropped-out,
   * such that the output domain is `in.domain() - filter.window()`.
   */
  template <typename TRaster>
  Raster<Value, TRaster::Dimension> crop(const TRaster& in) const
  {
    // FIXME check region is a Box
    const auto region = Linx::box(in.domain()) - Linx::box(m_window);
    Raster<Value, TRaster::Dimension> out(region.shape());
    transform_monolith(in.patch(region), out);
    return out;
  }

  /**
   * @brief Filter a box-based patch.
   * 
   * If input values from outside the raster domain are required,
   * then `in` must be an extrapolator.
   * On the contrary, if the box of `in` is small enough so that no extrapolated values are required,
   * then `in` can be a raw patch.
   * 
   * The output raster has the same shape as the box.
   */
  template <typename TPatch>
  Raster<Value, TPatch::Dimension> box(const TPatch& in) const
  {
    // FIXME check region is a Box
    Raster<Value, TPatch::Dimension> out(in.domain().shape());
    transform(in, out);
    return out;
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
  template <typename TPatch>
  Raster<Value, TPatch::Dimension> decimate(const TPatch& in) const
  { // FIXME adjust?

    const auto& raw = dont_extrapolate(in);
    const auto& front = in.domain().front();
    const auto& step = in.domain().step();
    Raster<Value, TPatch::Dimension> out(in.domain().shape());

    const auto grid_to_box = [&](const auto& g) {
      auto f = g.front() - front;
      for (std::size_t i = 0; i < f.size(); ++i) { // FIXME simplify with Linx
        f[i] /= step[i];
      }
      return Box<Dimension>::from_shape(f, g.shape());
    };

    const auto box = Internal::BorderedBox<Dimension>(rasterize(in).domain(), m_window);
    box.apply_inner_border(
        [&](const auto& ib) {
          const auto insub = raw.patch(ib);
          auto outsub = out.patch(grid_to_box(insub.domain()));
          transform_monolith(insub, outsub);
        },
        [&](const auto& ib) {
          const auto insub = in.patch(ib);
          auto outsub = out.patch(grid_to_box(insub.domain()));
          transform_monolith(insub, outsub);
        });

    return out;
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
    if constexpr (is_extrapolator<TIn>()) {
      transform_splits(in, out);
    } else {
      // FIXME check no extrapolation is required
      transform_monolith(in, out);
    }
  }

private:

  /**
   * @brief Filter by splitting inner and border regions.
   */
  template <typename TIn, typename TOut>
  void transform_splits(const TIn& in, TOut& out) const
  {
    const auto& raw = dont_extrapolate(in);
    const auto box = Internal::BorderedBox<Dimension>(Linx::box(raw.domain()), m_window);
    box.apply_inner_border(
        [&](const auto& ib) {
          const auto insub = raw.patch(ib);
          auto outsub = out.patch(insub.domain());
          transform_monolith(insub, outsub);
        },
        [&](const auto& ib) {
          const auto insub = in.patch(ib);
          auto outsub = out.patch(insub.domain());
          transform_monolith_extrapolator(insub, outsub);
        });
  }

  /**
   * @brief Filter a monolithic patch (no region splitting).
   */
  template <typename TIn, typename TOut>
  void transform_monolith(const TIn& in, TOut& out) const
  {
    auto patch = in.parent().patch(m_window);
    auto out_it = out.begin();
    for (const auto& p : in.domain()) {
      patch.translate(p);
      *out_it = reinterpret_cast<const TDerived&>(*this)(patch);
      ++out_it;
      patch.translate_back(p);
    }
  }

  template <typename TIn, typename TOut>
  void transform_monolith_extrapolator(const TIn& in, TOut& out) const
  {
    const auto extrapolated = in.parent().copy(Linx::box(in.domain()) + m_window);
    const auto box = extrapolated.domain() - m_window; // FIXME region - m_window.front()?
    transform_monolith(extrapolated.patch(box), out);
  }

protected:

  /**
   * @brief The window with origin at position 0.
   */
  TWindow m_window;
};

} // namespace Linx

#endif
