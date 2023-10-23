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
 * @brief Detect cosmic rays.
 * @param in The input 2D raster
 * @param pfa The detection probability of false alarm
 * 
 * This is a simple adaptive Laplacian thresholding.
 * The input raster is convolved with some Laplacian kernel.
 * The parameters of the background noise (empirically assumed Laplace-distributed)
 * of the filtered image are estimated to deduce the detection threshold from a PFA.
 */
template <typename TIn>
Raster<char> detect(const TIn& in, float pfa)
{
  const auto laplacian =
      convolution(Raster<float>(
          {3, 3},
          {-1. / 6., -2. / 3., -1. / 6., -2. / 3., 10. / 3, -2. / 3., -1. / 6., -2. / 3., -1. / 6.})) *
      extrapolate<NearestNeighbor>(in);

  // Empirically assume Laplace distribution
  float b = 0;
  for (const auto& e : laplacian) {
    b += std::abs(e);
  }
  const auto threshold = -b / laplacian.size() * std::log(2.0 * pfa);

  Raster<char> out(in.shape());
  out.generate(
      [=](auto c) {
        return c > threshold;
      },
      laplacian);
  return out;
}

/**
 * @brief Compute the minimum intensity distance between a point and its neighbors in a mask.
 */
template <typename TIn, typename TMask>
float distance(const TIn& input, const TMask& mask, const Position<2>& p)
{
  auto d = std::numeric_limits<float>::max();
  for (const auto& q : Box<2>::from_center(1, p) & input.domain()) { // FIXME optimize
    if (q != p && mask[q]) {
      auto delta = std::abs((input[q] - input[p]) / input[q]);
      if (delta < d) {
        d = delta;
      }
    }
  }
  return d;
}

/**
 * @brief Compute a sigma-clipping threshold.
 * @param abs The absolute values of the samples
 * @param factor The factor to apply
 * 
 * Assuming a centered distribution, compute sigma as 1.48 * MAD.
 */
template <typename TIn>
float sigma_clip(const TIn& abs, float factor)
{
  TIn nonconst = abs;
  auto it = nonconst.begin() + nonconst.size() / 2;
  std::nth_element(nonconst.begin(), nonconst.end(), it);
  return factor * 1.48 * *it;
}

/**
 * @brief Segment detected cosmic rays.
 * @param mask The detection map
 * @param threshold The distance threshold
 * 
 * Given a detection map, neighbors of flagged pixels are considered as candidate cosmic rays.
 * Some relative intensity distance is computed in the neighborhood
 * in order to decide whether the cadidate belongs to the cosmic ray or to the background by thresholding.
 */
template <typename TIn, typename TMask>
void segment(const TIn& input, TMask& mask, float threshold)
{
  auto candidates = dilation<typename TMask::Value>(Box<2>::from_center(1)) * extrapolate(mask, '\0') - mask;
  for (const auto& p : candidates.domain()) {
    if (candidates[p]) {
      auto d = distance(input, mask, p);
      if (d < threshold) {
        mask[p] = true;
      }
    }
  }
}

} // namespace Cosmics
} // namespace Linx

#endif
