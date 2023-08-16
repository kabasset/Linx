// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXRUN_COSMICS_H
#define _LINXRUN_COSMICS_H

#include "Linx/Data/Raster.h"
#include "Linx/Transforms/Kernel.h" // FIXME Filtering
#include "Linx/Transforms/StructuringElement.h" // FIXME Morphology

namespace Linx {

/**
 * @brief Detect cosmic rays and flag affected pixels.
 * @param in The input 2D raster
 * @param factor The detection threshold factor relative to the noise standard deviation
 * @param out The output flag map
 * @param flag The flag value
 * 
 * @see Umbaugh, Scott E. (2011). Digital image processing and analysis (2nd ed.).
 */
template <typename TIn, typename TOut>
void flag_cosmics_to(const TIn& in, double factor, TOut& out, typename TOut::value_type flag = true) {

  Raster<typename TIn::Value, 3> kernel(
      {3, 3, 4},
      {
          -1.0, -2.0, -1.0, +2.0, +4.0, +2.0, -1.0, -2.0, -1.0, // 0 deg
          -1.0, +2.0, -1.0, -2.0, +4.0, -2.0, -1.0, +2.0, -1.0, // 90 deg
          -1.0, -1.5, +2.0, -1.5, +4.0, -1.5, +2.0, -1.5, -1.0, // 45 deg
          +2.0, -1.5, -1.0, -1.5, +4.0, -1.5, -1.0, -1.5, 2.0 // 135 deg
      });

  // auto convolved = convolution(kernel) * extrapolate<NearestNeighbor>(in);
  // FIXME from xvalue?
  // FIXME broadcasting

  auto extrapolation = extrapolate<NearestNeighbor>(in);
  Raster<typename TIn::Value, 3> convolved({in.shape()[0], in.shape()[1], 4});
  for (long i = 0; i < 4; ++i) {
    auto k = kernel.section(i);
    auto c = convolved.section(i);
    convolution(k).transform(extrapolation, c);
  }

  const auto mean = 0.0;
  const auto var =
      std::inner_product(convolved.begin(), convolved.end(), convolved.begin(), 0.0) / (convolved.size() - 1);
  const auto threshold = factor * std::sqrt(var) + mean;

  out.apply(
      [=](auto f, int c0, int c1, int c2, int c3) {
        return c0 > threshold || c1 > threshold || c2 > threshold || c3 > threshold ? flag : f;
      },
      convolved.section(0),
      convolved.section(1),
      convolved.section(2),
      convolved.section(3));
}

/**
 * @brief Detect cosmic rays and return a binary flag.
 * @tparam T The flag value type
 * @param in A 2D raster
 * @param factor The detection threshold factor relative to the noise standard deviation
 */
template <typename T, typename TIn>
Raster<T> flag_cosmics(const TIn& in, double factor) {
  Raster<T> out(in.shape());
  flag_cosmics_to(in, factor, out);
  return out;
}

/**
 * @brief Perform morphological closing in place.
 */
template <typename TIn>
void close_flag(TIn& in, long radius = 1) {
  // auto strel = Mask<2>::ball<2>(radius); // FIXME accept masks in StructuringElement
  auto strel = Box<2>::from_center(radius);
  auto dilated = dilation<typename TIn::Value>(strel) * extrapolate<NearestNeighbor>(in);
  erosion<typename TIn::Value>(strel).transform(extrapolate<NearestNeighbor>(dilated), in);
}

} // namespace Linx

#endif
