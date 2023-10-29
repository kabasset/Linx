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
template <typename TIn, typename TPsf>
Raster<char> detect(const TIn& in, const TPsf& psf, float pfa)
{
  const auto laplacian_filter = convolution(
      Raster<float>({3, 3}, {-1. / 6., -2. / 3., -1. / 6., -2. / 3., 10. / 3, -2. / 3., -1. / 6., -2. / 3., -1. / 6.}));

  auto laplacian = laplacian_filter * extrapolate<NearestNeighbor>(in);
  Fits("/tmp/cosmic.fits").write(laplacian, 'a');

  const auto psf_filter = correlation(psf);
  auto stars = dilation<typename TPsf::Value>(Box<2>::from_center(1)) *
      extrapolate<NearestNeighbor>(psf_filter * extrapolate<NearestNeighbor>(in));
  Fits("/tmp/cosmic.fits").write(stars, 'a');

  const auto psf_laplacian = laplacian_filter * extrapolate(psf, 0.F); // FIXME optimize: compute only at center
  const auto center_pos = -psf_filter.window().front();
  const auto psf_laplacian_center = psf_laplacian[center_pos];
  const auto psf_center = psf[center_pos];
  printf("Central PSF Laplacian: %f\n", psf_laplacian_center);
  printf("Central PSF value: %f\n", psf_center);

  laplacian -= stars * psf_laplacian_center;
  Fits("/tmp/cosmic.fits").write(laplacian, 'a');

  // Empirically assume Laplace distribution
  const auto threshold = -norm<1>(laplacian) / laplacian.size() * std::log(2.0 * pfa);
  printf("Threshold: %f\n", threshold);

  Raster<char> out(in.shape());
  out.generate(
      [=](auto c) {
        return c > threshold;
      },
      laplacian);
  return out;
}

/**
 * @brief Compute the minimum contrast between a point and its neighbors in a mask.
 * 
 * The contrast is negative when the point intensity is higher than that of the neighborhood.
 */
template <typename TIn, typename TMask>
float min_contrast(const TIn& in, const TMask& mask, const Position<2>& p)
{
  auto out = std::numeric_limits<float>::max();
  const Position<2> dx {0, 1};
  const Position<2> dy {1, 0};
  for (const auto& neighbor : {p - dx, p + dx, p - dy, p + dy}) { // FIXME optimize?
    if (mask[neighbor]) {
      auto contrast = (in[neighbor] - in[p]) / in[neighbor]; // FIXME assumes in > 0
      if (contrast < out) {
        out = contrast;
      }
    }
  }
  return out;
}

/**
 * @brief Segment detected cosmic rays.
 * @param mask The detection map
 * @param threshold The similarity threshold
 * 
 * Given a detection map, neighbors of flagged pixels are considered as candidate cosmic rays.
 * Some similarity distance is computed in the neighborhood in order to decide
 * whether the cadidate belongs to the cosmic ray or to the background, by thresholding.
 */
template <typename TIn, typename TMask>
void segment(const TIn& in, TMask& mask, float threshold)
{
  // FIXME Mask<2>::ball<1>(1)
  auto candidates = dilation<typename TMask::Value>(Box<2>::from_center(1)) * extrapolate(mask, '\0') - mask;
  for (const auto& p : candidates.domain() - Box<2>::from_center(1)) {
    if (candidates[p]) {
      if (min_contrast(in, mask, p) < threshold) {
        mask[p] = true;
      }
    }
  }
}

} // namespace Cosmics
} // namespace Linx

#endif
