// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXRUN_COSMICS_H
#define _LINXRUN_COSMICS_H

#include "Linx/Data/Raster.h"
#include "Linx/Transforms/Kernel.h" // FIXME Filtering
#include "Linx/Transforms/StructuringElement.h" // FIXME Morphology

namespace Linx {
namespace Cosmics {

/**
 * @brief Detect cosmic rays and flag affected pixels.
 * @param in The input 2D raster
 * @param factor The detection threshold factor relative to the noise standard deviation
 * @param out The output flag map
 * @param flag The flag value
 * 
 * This is a simple adaptive Laplacian thresholding.
 * The input raster is convolved with some Laplacian kernel.
 * The Gaussian noise parameters of the result are estimated to deduce the detection threshold.
 * 
 * @see Another, non isotropic approach: Umbaugh, Scott E. (2011). Digital image processing and analysis (2nd ed.).
 */
template <typename TIn, typename TOut>
void flag_to(const TIn& in, double factor, TOut& out, typename TOut::Value flag = true)
{
  using T = typename TIn::Value;

  const auto laplacian = convolution(Raster<T>({3, 3}, {-.25, -.5, -.25, -.5, 3, -.5, -.25, -.5, -.25})) *
      extrapolate<NearestNeighbor>(in);

  const auto mean = 0.0;
  const auto var =
      std::inner_product(laplacian.begin(), laplacian.end(), laplacian.begin(), 0.0) / (laplacian.size() - 1);
  const auto threshold = factor * std::sqrt(var) + mean;

  out.apply(
      [=](auto f, auto c) {
        return c > threshold ? flag : f;
      },
      laplacian);
}

/**
 * @brief Detect cosmic rays and return a binary flag.
 * @tparam T The flag value type
 * @param in A 2D raster
 * @param factor The detection threshold factor relative to the noise standard deviation
 */
template <typename T, typename TIn>
Raster<T> flag(const TIn& in, double factor)
{
  Raster<T> out(in.shape());
  flag_to(in, factor, out);
  return out;
}

/**
 * @brief Perform morphological closing in place.
 */
template <typename TIn>
void close_flagmap(TIn& in, long radius = 1)
{
  using T = typename TIn::Value;
  // auto strel = Mask<2>::ball<2>(radius); // FIXME accept masks in StructuringElement
  auto strel = Box<2>::from_center(radius);
  auto dilated = dilation<T>(strel) * extrapolate<NearestNeighbor>(in);
  erosion<T>(strel).transform(extrapolate<NearestNeighbor>(dilated), in);
}

/**
 * @brief Dilate the flag map.
 */
template <typename TIn>
auto dilate_flagmap(const TIn& in, long radius = 1)
{
  if (radius == 0) {
    return in;
  }
  using T = typename TIn::Value;
  return dilation<T>(Box<2>::from_center(radius)) * extrapolate<NearestNeighbor>(in);
}

} // namespace Cosmics
} // namespace Linx

#endif
