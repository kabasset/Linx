// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_MIXINS_FILTER_H
#define _LINXTRANSFORMS_MIXINS_FILTER_H

#include "Linx/Data/BorderedBox.h"
#include "Linx/Data/Box.h"
#include "Linx/Data/Grid.h"
#include "Linx/Data/Raster.h"
#include "Linx/Transforms/Extrapolation.h"

namespace Linx {

/**
 * @ingroup filtering
 * @brief Spatial filtering mixin.
 * 
 * Child classes must implement `window()` and `transform(in, out)`.
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
   * @brief Get the filter window.
   */
  inline decltype(auto) window() const
  {
    return LINX_CRTP_CONST_DERIVED.window_impl();
  }

  /**
   * @brief Compute the impulse response of the filter.
   */
  auto impulse() const
  {
    const auto& w = window();
    const auto o = -w.front();
    auto raster = Raster<Value, Dimension>(w.shape());
    raster[o] = Value(1); // FIXME or back-o?
    return *this * extrapolation(raster);
  }

  /**
   * @brief Apply the filter into a given output.
   */
  template <typename TIn, typename TOut>
  inline void transform(const TIn& in, TOut& out) const
  {
    // FIXME make applicable to Sequence<Position>?
    LINX_CRTP_CONST_DERIVED.transform_impl(in, out);
  }

  /**
   * @brief Apply the filter with cropping.
   */
  template <typename U, Index N, typename UHolder>
  Raster<Value, N> operator*(const Raster<U, N, UHolder>& in) const
  {
    const auto& w = box(window());
    const auto shape = in.shape() - extend<N>(w.shape() - 1);
    Raster<Value, N> out(shape);
    transform(in, out);
    return out;
  }

  /**
   * @brief Apply the filter with extrapolation.
   */
  template <typename URaster, typename UMethod>
  Raster<Value, URaster::Dimension> operator*(const Extrapolation<URaster, UMethod>& in) const
  {
    Raster<Value, URaster::Dimension> out(in.shape());
    transform(in, out);
    return out;
  }

  /**
   * @brief Apply the filter to a box-, line- or grid-based patch.
   */
  template <typename U, typename UParent, typename URegion>
  Raster<Value, URegion::Dimension> operator*(const Patch<U, UParent, URegion>& in) const
  {
    // URegion::Dimension is not defined for Sequence
    Raster<Value, URegion::Dimension> out(in.domain().shape()); // Box or Grid
    // FIXME support arbitrary patches
    transform(in, out);
    return out;
  }

  /**
   * @brief Apply the filter to a single pixel.
   */
  template <typename U, typename UParent>
  Value operator*(const Patch<U, UParent, Position<UParent::Dimension>>& in) const
  {
    Raster<Value, UParent::Dimension, StdHolder<std::array<Value, 1>>> out(Position<UParent::Dimension>::one());
    const auto patch = in.parent()(Box<UParent::Dimension>(in.domain(), in.domain())); // Position to Box
    transform(patch, out);
    return out[0];
  }

  /**
   * @brief Apply the filter to a sequence of pixels.
   */
  template <typename U, typename UParent, typename UHolder>
  Sequence<Value> operator*(const Patch<U, UParent, Sequence<Position<UParent::Dimension>, UHolder>>& in) const
  {
    Sequence<Value> out(in.size());
    std::transform(in.domain().begin(), in.domain().end(), out.begin(), [&](const auto& p) {
      return (*this) * in.parent()(p);
    });
    return out;
  }
};

/**
 * @ingroup filtering
 * @brief Test whether a class is a filter, i.e. implements `FilterMixin`.
 */
template <typename T>
constexpr bool is_filter()
{
  return is_base_template_of<FilterMixin, T>();
}

} // namespace Linx

#endif
