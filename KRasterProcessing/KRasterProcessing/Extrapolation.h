// Copyright (C) 2022, CNES
// This file is part of PhiFun <github.com/kabasset/PhiFun>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _KRASTERPROCESSING_EXTRAPOLATION_H
#define _KRASTERPROCESSING_EXTRAPOLATION_H

namespace Kast {

template <typename TBoundary>
class Extrapolator {

public:
  Extrapolator(long width, long height, const OutOfBoundsError& boundary) :
      m_width(width), m_height(height), m_boundary(boundary) {}

  template <typename TRaster, typename... TLongs>
  const typename TRaster::value_type* bottomLeft(const TRaster& raster, double x, double y, TLongs... coords) const {
    if (outside(x, y)) {
      return outOfBoundsValue(center);
    }
    return raster[{long(x), long(y), coords...}];
  }

  template <typename T, typename... TLongs>
  const T* right(const T* center, double x, double y, TLongs... coords) const {
    if (outside(x + 1, y)) {
      return outOfBoundsValue(center);
    }
    return center + 1;
  }

  template <typename TRaster, typename... TLongs>
  const typename TRaster::value_type* top(const TRaster& raster, double x, double y, TLongs... coords) const {
    if (outside(x, y + 1)) {
      return outOfBoundsValue(center);
    }
    return center + m_width;
  }

  template <typename T, typename... TLongs>
  const T* left(const T* center, double x, double y, TLongs... coords) const {
    if (outside(x - 1, y)) {
      return outOfBoundsValue(center);
    }
    return center - 1;
  }

  template <typename TRaster, typename... TLongs>
  const typename TRaster::value_type* bottom(const TRaster& raster, double x, double y, TLongs... coords) const {
    if (outside(x, y - 1)) {
      return outOfBoundsValue(center);
    }
    return center - m_width;
  }

private:
  bool outside(double x, double y) {
    return x < 0 || x > m_width - 1 || y < 0 || y > m_height - 1;
  }

  long m_width;
  long m_height;
  const OutOfBoundsError& boundary;
};

/**
 * @brief Specialization for `OutOfBoundsError`.
 */
template <>
class Extrapolator<OutOfBoundsError> {

public:
  Extrapolator(long width, long height, const OutOfBoundsError& boundary) :
      m_width(width), m_height(height), m_boundary(boundary) {}

  template <typename TRaster, typename... TLongs>
  const typename TRaster::value_type* bottomLeft(const TRaster& raster, double x, double y, TLongs... coords) const {
    mayThrow(x, y);
    return raster[{long(x), long(y), coords...}];
  }

  template <typename T, typename... TLongs>
  const T* right(const T* center, double x, double y, TLongs... coords) const {
    mayThrow(x + 1, y);
    return center + 1;
  }

  template <typename TRaster, typename... TLongs>
  const typename TRaster::value_type* top(const TRaster& raster, double x, double y, TLongs... coords) const {
    mayThrow(x, y + 1);
    return center + m_width;
  }

  template <typename T, typename... TLongs>
  const T* left(const T* center, double x, double y, TLongs... coords) const {
    mayThrow(x - 1, y);
    return center - 1;
  }

  template <typename TRaster, typename... TLongs>
  const typename TRaster::value_type* bottom(const TRaster& raster, double x, double y, TLongs... coords) const {
    mayThrow(x, y - 1);
    return center - m_width;
  }

private:
  bool outside(double x, double y) {
    return x < 0 || x > m_width - 1 || y < 0 || y > m_height - 1;
  }

  long m_width;
  long m_height;
  const OutOfBoundsError& boundary;
};

/**
 * @brief Specialization for `Constant`.
 */
template <typename T>
class Extrapolator<Constant<T>> {

public:
  Extrapolator(long width, long height, const Constant<T>& boundary) :
      m_width(width), m_height(height), m_boundary(boundary) {}

  template <typename TRaster, typename... TLongs>
  const typename TRaster::value_type* bottomLeft(const TRaster& raster, double x, double y, TLongs... coords) const {
    if (outside(x, y)) {
      return &m_boundary.value;
    }
    return raster[{long(x), long(y), coords...}];
  }

  template <typename T, typename... TLongs>
  const T* right(const T* center, double x, double y, TLongs... coords) const {
    if (outside(x + 1, y)) {
      return &m_boundary.value;
    }
    return center + 1;
  }

  template <typename TRaster, typename... TLongs>
  const T* top(const T* center, double x, double y, TLongs... coords) const {
    if (outside(x, y + 1)) {
      return &m_boundary.value;
    }
    return center + m_width;
  }

  template <typename T, typename... TLongs>
  const T* left(const T* center, double x, double y, TLongs... coords) const {
    if (outside(x - 1, y)) {
      return &m_boundary.value;
    }
    return center - 1;
  }

  template <typename TRaster, typename... TLongs>
  const T* bottom(const T* center, double x, double y, TLongs... coords) const {
    if (outside(x, y - 1)) {
      return &m_boundary.value;
    }
    return center - m_width;
  }

private:
  bool outside(double x, double y) {
    return x < 0 || x > m_width - 1 || y < 0 || y > m_height - 1;
  }

  void mayThrow(double x, double y) {
    if (outside(x, y)) {
      throw OutOfBoundsError();
    }
  }

  long m_width;
  long m_height;
  const Constant<T>& boundary;
};

/**
 * @brief Specialization for `NearestNeighbor`.
 */
template <typename T>
class Extrapolator<NearestNeighbor> {

public:
  Extrapolator(long width, long height, const NearestNeighbor& boundary) :
      m_width(width), m_height(height), m_boundary(boundary) {}

  template <typename TRaster, typename... TLongs>
  const typename TRaster::value_type* bottomLeft(const TRaster& raster, double x, double y, TLongs... coords) const {
    const auto left = clamp<long>(x, 0, m_width);
    const auto right = clamp<long>(y, 0, m_height);
    return raster[{left, right, coords...}];
  }

  template <typename T, typename... TLongs>
  const T* right(const T* center, double x, double y, TLongs... coords) const {
    if (x > m_width) {
      return center;
    }
    return center + 1;
  }

  template <typename TRaster, typename... TLongs>
  const T* top(const T* center, double x, double y, TLongs... coords) const {
    if (y > m_height) {
      return center;
    }
    return center + m_width;
  }

  template <typename T, typename... TLongs>
  const T* left(const T* center, double x, double y, TLongs... coords) const {
    if (x < 0) {
      return center;
    }
    return center - 1;
  }

  template <typename TRaster, typename... TLongs>
  const T* bottom(const T* center, double x, double y, TLongs... coords) const {
    if (y < 0) {
      return center;
    }
    return center - m_width;
  }

private:
  template <typename T>
  T clamp(T in, T min, T max) {
    return std::max(min, std::min(in, max));
  }

  long m_width;
  long m_height;
  const NearestNeighbor& boundary;
};

} // namespace Kast

#endif // _KRASTERPROCESSING_EXTRAPOLATION_H
