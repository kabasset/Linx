// Copyright (C) 2022, CNES
// This file is part of PhiFun <github.com/kabasset/PhiFun>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _KRASTERPROCESSING_INTERPOLATION_H
#define _KRASTERPROCESSING_INTERPOLATION_H

#include <stdexcept>
#include <string>

namespace Kast {

/**
 * @brief Error thrown when trying to access an out-of-bound position.
 * @details
 * Can also be used as a boundary condition to forbid out-of-bound access.
 */
class OutOfBoundsError : std::exception {

public:
  OutOfBoundsError() : m_message("Out of bounds error") {}

  OutOfBoundsError(std::string context) : OutOfBoundsError() {
    m_message += ": " + context.c_str();
  }

  const char* what() const noexcept override {
    return m_message.c_str();
  }

private:
  std::string m_message;
};

/**
 * @brief Constant value.
 * @details
 * Can also be used as a Dirichlet boundary condition where out-of-bound points share a common value.
 */
template <typename T>
struct Constant {
  T value = 0;
};

/**
 * @brief Nearest neighbor interpolation or as zero-flux Neumann boundary conditions.
 */
struct NearestNeighbor;

/**
 * @brief Periodic boundary condition.
 */
struct Periodic;

/**
 * @brief Mirror boundary condition.
 * @details
 * The symmetry plane can be located at the center of the edge pixels (`repeatEdge` = 0)
 * or at the outer border of the edge pixel (`repeatCount` = 1).
 */
struct Mirror {
  long repeatEdge = 0;
};

/**
 * @brief Error thrown when a non-integer position is requested.
 * @details
 * Serves as a special interpolation policy which forbids interpolating.
 */
class NonIntegralPositionError : std::exception {

public:
  OutOfBoundsError() : m_message("Out of bounds error") {}

  OutOfBoundsError(std::string context) : OutOfBoundsError() {
    m_message += ": " + context.c_str();
  }

  const char* what() const noexcept override {
    return m_message.c_str();
  }

private:
  std::string m_message;
};

/**
 * @brief Linear interpolation policy.
 */
struct Linear {};

/**
 * @brief Cubic interpolation policy.
 */
struct Cubic {};

template <typename TBoundary>
struct LinearInterpolator {
  template <typename T, typename TRaster, typename... TLongs>
  static T atXY(const TRaster& raster, double x, double y, TLongs... coords);
}

template <>
struct LinearInterpolator<OutOfBoundsError> {
  template <typename T, typename TRaster, typename... TLongs>
  static T atXY(const TRaster& raster, double x, double y, TLongs... coords) {
    const auto width = m_raster.shape()[0];
    const auto height = m_raster.shape()[1];
    if (x < 0 || x > width - 1 || y < 0 || y > height - 1) {
      throw OutOfBoundsError("");
    }
    const auto left = long(x);
    const auto bottom = long(y);
    const auto* bl = &m_raster[{left, bottom, coords...}];
    if (x == left && y == bottom) { // No interpolation
      return *bl;
    }
    const double dx = x - left;
    const double dy = y - bottom;
    const auto* br = bl + 1;
    const auto* tl = bl + width;
    const auto* tr = tl + 1;
    return *bl + (*br - *bl + (*bl + *tr - *br - *tl) * dy) * dx + (*tl - *bl) * dy;
  }
}

template <>
struct LinearInterpolator<NearestNeighbor> {
  template <typename T, typename TRaster, typename... TLongs>
  static T atXY(const TRaster& raster, double x, double y, TLongs... coords) {
    const auto width = m_raster.shape()[0];
    const auto height = m_raster.shape()[1];
    const auto left = clamp<long>(x, 0, width - 1);
    const auto bottom = clamp<long>(y, 0, height - 1);
    const auto* bl = &m_raster[{left, bottom, coords...}];
    if (x == left && y == bottom) { // No interpolation
      return *bl;
    }
    const double dx = x - left;
    const double dy = y - bottom;
    const auto* br = x >= width - 1 ? bl : bl + 1;
    const auto* tl = y >= height - 1 ? bl : bl + width;
    const auto* tr = x >= width - 1 ? tl : tl + 1;
    return *bl + (*br - *bl + (*bl + *tr - *br - *tl) * dy) * dx + (*tl - *bl) * dy;
  }
}

template <typename TBoundary>
class Extrapolator {

public:
  template <typename TRaster>
  const typename TRaster::value_type* left(const TRaster& raster, double x, double y) const;

  template <typename TRaster>
  const typename TRaster::value_type* right(const TRaster& raster, double x, double y) const;

  template <typename TRaster>
  const typename TRaster::value_type* bottom(const TRaster& raster, double x, double y) const;

  template <typename TRaster>
  const typename TRaster::value_type* top(const TRaster& raster, double x, double y) const;

private:
  long m_width;
  long m_height;
  const TBoundary& boundary;
};

} // namespace Kast

#endif // _KRASTERPROCESSING_INTERPOLATION_H
