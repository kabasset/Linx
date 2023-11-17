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
 * @brief Pearson correlation coefficient functor.
*/
template <typename T>
class PearsonCorrelation {
public:

  using Value = T;

  template <typename... TArgs>
  PearsonCorrelation(TArgs&&... args) : m_template(std::forward<TArgs>(args)...), m_sum2()
  {
    const auto mean = std::accumulate(m_template.begin(), m_template.end(), Value()) / m_template.size();
    std::transform(m_template.begin(), m_template.end(), m_template.begin(), [=](auto& e) {
      return e - mean;
    });
    m_sum2 = std::inner_product(m_template.begin(), m_template.end(), m_template.begin(), Value());
  }

  Value operator()(const std::vector<Value>& neighbors) const // FIXME nonconst? member?
  {
    const auto mean = std::accumulate(neighbors.begin(), neighbors.end(), Value()) / neighbors.size();
    auto centered = neighbors;
    std::transform(centered.begin(), centered.end(), centered.begin(), [=](auto& e) {
      return e - mean;
    });
    const auto sum2 = std::inner_product(centered.begin(), centered.end(), centered.begin(), Value());
    return std::inner_product(m_template.begin(), m_template.end(), centered.begin(), Value()) /
        std::sqrt(m_sum2 * sum2);
  }

private:

  std::vector<Value> m_template;
  Value m_sum2;
};

/**
 * @brief Quotient filter, i.e. minimum value of the ratio between neighbors and template, normalized.
*/
template <typename T>
class QuotientFilter {
public:

  using Value = T;

  template <typename... TArgs>
  QuotientFilter(TArgs&&... args) : m_template(std::forward<TArgs>(args)...)
  {}

  Value operator()(const std::vector<Value>& neighbors) const
  {
    Value out = std::numeric_limits<Value>::max();
    Value norm2 = 0;
    for (std::size_t i = 0; i < m_template.size(); ++i) {
      const auto q = neighbors[i] / m_template[i]; // FIXME rm offset?
      norm2 += q * q;
      if (q < out) { // FIXME more robust, median?
        out = q;
      }
    }
    return out * std::sqrt(m_template.size() / norm2);
  }

private:

  std::vector<Value> m_template;
};

template <typename TIn, typename TPsf>
Raster<typename TPsf::Value> quotient(const TIn& in, const TPsf& psf)
{
  using T = typename TPsf::Value;
  auto filter = StructuringElement<QuotientFilter<T>, Box<2>>(
      QuotientFilter<T>(psf.begin(), psf.end()),
      psf.domain() - (psf.shape() - 1) / 2);
  return filter * extrapolate<NearestNeighbor>(in);
}

template <typename TIn, typename TPsf>
Raster<typename TPsf::Value> match(const TIn& in, const TPsf& psf)
{
  using T = typename TPsf::Value;
  auto filter = StructuringElement<PearsonCorrelation<T>, Box<2>>(
      PearsonCorrelation<T>(psf.begin(), psf.end()),
      psf.domain() - (psf.shape() - 1) / 2);
  return filter * extrapolate<NearestNeighbor>(in);
}

template <typename TIn>
Raster<typename TIn::Value> laplacian(const TIn& in)
{
  using T = typename TIn::Value;
  const auto filter = convolution(
      Raster<T>({3, 3}, {-1. / 6., -2. / 3., -1. / 6., -2. / 3., 10. / 3, -2. / 3., -1. / 6., -2. / 3., -1. / 6.}));
  return filter * extrapolate<NearestNeighbor>(in);
}

template <typename TIn>
Raster<typename TIn::Value> dilate(const TIn& in, Index radius = 1)
{
  using T = typename TIn::Value;
  auto filter = dilation<T>(Box<2>::from_center(radius)); // FIXME L2-ball
  return filter * extrapolate<NearestNeighbor>(in);
}

template <typename TIn>
Raster<typename TIn::Value> blur(const TIn& in, Index radius = 1)
{
  using T = typename TIn::Value;
  auto filter = mean_filter<T>(Box<2>::from_center(radius)); // FIXME L2-ball
  return filter * extrapolate<NearestNeighbor>(in);
}

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
Raster<char> detect(const TIn& in, const TPsf& psf, float pfa, float tq)
{
  auto laplacian_map = laplacian(in);
  Fits("/tmp/cosmic.fits").write(laplacian_map, 'a');

  // Empirically assume Laplace distribution
  const auto tl = -norm<1>(laplacian_map) / laplacian_map.size() * std::log(2.0 * pfa);
  printf("Threshold: %f\n", tl);

  const Index radius = std::sqrt(psf.size()) / 4;
  printf("Radius: %li\n", radius);
  auto quotient_map = dilate(quotient(in, psf), radius);
  Fits("/tmp/cosmic.fits").write(quotient_map, 'a');

  Raster<char> out(in.shape());
  out.generate(
      [=](auto l, auto q) {
        return l > tl && q < tq; // FIXME compute quotient only where l > tl
      },
      laplacian_map,
      quotient_map);
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
