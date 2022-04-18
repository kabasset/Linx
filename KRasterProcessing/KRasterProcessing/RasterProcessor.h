// Copyright (C) 2022, CNES
// This file is part of PhiFun <github.com/kabasset/PhiFun>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _KRASTERPROCESSING_RASTERPROCESSOR_H
#define _KRASTERPROCESSING_RASTERPROCESSOR_H

#include "KRasterProcessing/Interpolation.h"

namespace KRasterProcessing {

/**
 * @brief A class to gather image processing functions for `Raster` objects.
 * @details
 * Functions which act on pixels intependently are covered by the `Raster` class already,
 * using `apply()` and `generate()`.
 * 
 * Policies are set with `boundary()` and `interpolation()`,
 * which create a new `RasterProcessor` for performance.
 * 
 * For example, to perform a linear interpolation at coordinates (x, y)
 * with nearest neighbor boundary conditions:
 * \code
 * auto proc = raster.processor().boundary(NearestNeighbor()).interpolation(Linear());
 * auto v = proc.atXY(x, y);
 * \endcode
 */
template <typename TRaster, typename TBoundary, typename TInterpolation>
class RasterProcessor {

public:
  /**
   * @brief Constructor.
   * @see `Raster::processor()` for a non-template shortcut.
   */
  RasterProcessor(TRaster& raster, TBoundary&& bc, TInterpolation&& f) :
      m_raster(raster), m_boundary(std::forward<TBoundary>(bc)), m_interpolation(std::forward<TInterpolation>(f)) {}

  /**
   * @brief Set the boundary conditions policy.
   */
  template <typename T>
  RasterProcessor<TRaster, T, TInterpolation> boundary(T&& bc) {
    return {m_raster, std::forward<T>(bc), std::move(m_interpolation)};
  }

  /**
   * @brief Set the interpolation policy.
   */
  template <typename T>
  RasterProcessor<TRaster, TBoundary, T> interpolation(T&& f) {
    return {m_raster, std::move(m_boundary), std::forward<T>(f)};
  }

  /**
   * @brief Compute interpolated value in (x, y) according to the processor policies.
   */
  template <typename T = typename TRaster::Value, typename... TLongs>
  inline T atXY(double x, double y, TLongs... coords) const;

private:
  TRaster& m_raster;
  TBoundary m_boundary;
  TInterpolation m_interpolation;
};

template <typename TRaster>
RasterProcessor<TRaster, OutOfBoundsError, NonIntegerPositionError> makeProcessor(TRaster& raster) {
  return {raster, {}, {}};
}

template <typename TRaster, typename TBoundary, typename TInterpolation>
template <typename T, typename... TLongs>
T RasterProcessor<TRaster, TBoundary, TInterpolation>::atXY(double x, double y, TLongs... coords) {}

} // namespace KRasterProcessing

#endif // _KRASTERPROCESSING_RASTERPROCESSOR_H
