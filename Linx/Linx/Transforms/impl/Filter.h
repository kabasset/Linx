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
 * @brief Create a vector of higher dimension.
 */
template <Index M, typename T, Index N>
Vector<T, M> extend(const Vector<T, N>& vector, Vector<T, M> padding = Vector<T, M>::zero())
{ // FIXME to Vector.h
  std::copy(vector.begin(), vector.end(), padding.begin());
  return padding;
}

/**
 * @brief Create a box of higher dimension.
 */
template <Index M, Index N>
Box<M> extend(const Box<N>& in, const Position<M>& padding = Position<M>::zero())
{ // FIXME to Box.h
  return {extend<M>(in.front(), padding), extend<M>(in.back(), padding)};
}

/**
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
   * @brief Compute the impulse response of the filter.
   */
  auto impulse() const
  {
    const auto& w = reinterpret_cast<const TDerived&>(*this).window();
    const auto o = -w.front();
    auto raster = Raster<Value, Dimension>(w.shape());
    raster[o] = Value(1); // FIXME or back-o?
    return *this * extrapolation(raster);
  }

  /**
   * @brief Apply the filter according to the input type.
   * 
   * This is equivalent to calling the overload of `transform()` suitable to `TIn` with an output of minimal shape.
   */
  template <typename TIn>
  Raster<Value, TIn::Dimension> operator*(const TIn& in) const
  {
    Raster<Value, TIn::Dimension> out;
    if constexpr (is_patch<TIn>()) {
      out = Raster<Value, TIn::Dimension>(in.domain().shape()); // Box or Grid
      // FIXME support arbitrary patches
    } else if constexpr (is_extrapolator<TIn>()) {
      out = Raster<Value, TIn::Dimension>(in.shape());
    } else {
      const auto& window = reinterpret_cast<const TDerived&>(*this).window();
      out = Raster<Value, TIn::Dimension>(in.shape() - extend<TIn::Dimension>(window.shape()));
    }
    reinterpret_cast<const TDerived&>(*this).transform(in, out);
    return out;
  }
};

} // namespace Linx

#endif
