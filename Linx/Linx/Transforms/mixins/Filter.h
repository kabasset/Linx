// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
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
  decltype(auto) window() const
  {
    return static_cast<const TDerived&>(*this).window_impl();
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
  void transform(const TIn& in, TOut& out) const
  {
    static_cast<const TDerived&>(*this).transform_impl(in, out);
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
   * @brief Apply the filter to a patch.
   */
  template <typename U, typename UParent, typename URegion>
  Raster<Value, UParent::Dimension> operator*(const Patch<U, UParent, URegion>& in) const
  {
    // FIXME support arbitrary patches
    Raster<Value, UParent::Dimension> out(in.domain().shape()); // Box or Grid
    transform(in, out);
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
