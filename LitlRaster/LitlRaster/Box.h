/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_BOX_H
#define _LITLRASTER_BOX_H

#include "LitlRaster/Region.h"

namespace Litl {

template <Index N>
using Box = Region<N>; // FIXME rename Region

/**
 * @brief Create a list of boxes around the box.
 * @param margin The extent of the surrounding
 * @details
 * The indices of `margin.front` must be negative or null
 * while those of `margin.back` must be positive or null.
 * No empty boxes are created, such that the number of output boxes
 * is less than `2 * in.dimension()` if some indices are null.
 * 
 * The union of all output boxes and the input box is a box such that:
 * `union.front = in.front + margin.front` and `union.back = in.back + margin.back`.
 * Partitioning is optimized for data locality when scanning raster pixels in the regions.
 */
template <Index N>
std::vector<Box<N>> surround(Box<N> in, const Box<N>& margin) { // FIXME in Box

  const auto dim = in.dimension();
  std::vector<Box<N>> out;
  out.reserve(dim * 2);

  for (Index i = 0; i < dim; ++i) {

    // Front
    const auto f = margin.front[i];
    if (f < 0) {
      auto before = in;
      before.back[i] = in.front[i] - 1;
      before.front[i] = in.front[i] += f;
      out.push_back(before);
    }

    // Back
    const auto b = margin.back[i];
    if (b > 0) {
      auto after = in;
      after.front[i] = in.back[i] + 1;
      after.back[i] = in.back[i] += b;
      out.push_back(after);
    }
  }

  return out;
}

template <Index N>
Box<N>& expand(Box<N>& in, const Box<N>& extent) { // FIXME operator+
  in.front += extent.front;
  in.back += extent.back;
  return in;
}

} // namespace Litl

#endif
