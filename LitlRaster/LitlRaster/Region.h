/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_REGION_H
#define _LITLRASTER_REGION_H

#include "LitlRaster/Vector.h"

namespace Litl {

/**
 * @brief A ND bounding box, defined by its front and back positions (both inclusive),
 * or front position and shape.
 * @details
 * Like `Position`, this class stores no pixel values, but coordinates.
 */
template <Index N = 2>
struct Region {
  /**
   * @brief Create a region from a front position and shape.
   */
  static Region<N> fromShape(Position<N> frontPosition, Position<N> shape) {
    Region<N> region {frontPosition, frontPosition};
    region.back += shape - 1;
    return region;
  }

  /**
   * @brief Create a region from a radius and center position.
   */
  static Region<N> fromCenter(Index radius = 1, const Position<N> center = Position<N>::zero()) {
    return Region<N> {center - radius, center + radius};
  }

  /**
   * @brief Create an unlimited region.
   * @details
   * Front and back bounds along each axis are respectively 0 and inf.
   */
  static Region<N> whole() {
    return {Position<N>::zero(), Position<N>::inf()};
  }

  /**
   * @brief Compute the region shape.
   */
  Position<N> shape() const {
    return back - front + 1;
  }

  /**
   * @brief Get the number of dimensions.
   */
  Index dimension() const {
    return front.size();
  }

  /**
   * @brief Compute the region size, i.e. number of pixels.
   */
  Index size() const {
    return shapeSize(shape());
  }

  /**
   * @brief The front position in the region.
   */
  Position<N> front;

  /**
   * @brief The back position in the region.
   */
  Position<N> back;
};

/**
 * @relates Region
 * @brief Check whether two regions are equal.
 */
template <Index N = 2>
bool operator==(const Region<N>& lhs, const Region<N>& rhs) {
  return lhs.front == rhs.front && lhs.back == rhs.back;
}

/**
 * @relates Region
 * @brief Check whether two regions are different.
 */
template <Index N = 2>
bool operator!=(const Region<N>& lhs, const Region<N>& rhs) {
  return lhs.front != rhs.front || lhs.back != rhs.back;
}

/**
 * @relates Region
 * @brief Add a position.
 */
template <Index N = 2>
Region<N>& operator+=(Region<N>& lhs, const Position<N>& rhs) {
  lhs.front += rhs;
  lhs.back += rhs;
  return lhs;
}

/**
 * @relates Region
 * @brief Subtract a position.
 */
template <Index N = 2>
Region<N>& operator-=(Region<N>& lhs, const Position<N>& rhs) {
  lhs.front -= rhs;
  lhs.back -= rhs;
  return lhs;
}

/**
 * @relates Region
 * @brief Add a scalar to each coordinate.
 */
template <Index N = 2>
Region<N>& operator+=(Region<N>& lhs, Index rhs) {
  lhs.front += rhs;
  lhs.back += rhs;
  return lhs;
}

/**
 * @relates Region
 * @brief Subtract a scalar to each coordinate.
 */
template <Index N = 2>
Region<N>& operator-=(Region<N>& lhs, Index rhs) {
  lhs.front -= rhs;
  lhs.back -= rhs;
  return lhs;
}

/**
 * @relates Region
 * @brief Add 1 to each coordinate.
 */
template <Index N = 2>
Region<N>& operator++(Region<N>& lhs) {
  lhs += 1;
  return lhs;
}

/**
 * @relates Region
 * @brief Subtract 1 to each coordinate.
 */
template <Index N = 2>
Region<N>& operator--(Region<N>& lhs) {
  lhs -= 1;
  return lhs;
}

/**
 * @relates Region
 * @brief Return the current region and then add 1 to each coordinate.
 */
template <Index N = 2>
Region<N> operator++(Region<N>& lhs, int) {
  auto res = lhs;
  ++lhs;
  return res;
}

/**
 * @relates Region
 * @brief Return the current region and then subtract 1 to each coordinate.
 */
template <Index N = 2>
Region<N> operator--(Region<N>& lhs, int) {
  auto res = lhs;
  --lhs;
  return res;
}

/**
 * @relates Region
 * @brief Identity.
 */
template <Index N = 2>
Region<N> operator+(const Region<N>& rhs) {
  return rhs;
}

/**
 * @relates Region
 * @brief Change the sign of each coordinate.
 */
template <Index N = 2>
Region<N> operator-(const Region<N>& rhs) {
  return {-rhs.front, -rhs.back};
}

/**
 * @relates Region
 * @brief Add a region and a position.
 */
template <Index N = 2>
Region<N> operator+(const Region<N>& lhs, const Position<N>& rhs) {
  auto res = lhs;
  res += rhs;
  return res;
}

/**
 * @relates Region
 * @brief Subtract a region and a position.
 */
template <Index N = 2>
Region<N> operator-(const Region<N>& lhs, const Position<N>& rhs) {
  auto res = lhs;
  res -= rhs;
  return res;
}

/**
 * @relates Region
 * @brief Add a region and a scalar.
 */
template <Index N = 2>
Region<N> operator+(const Region<N>& lhs, Index rhs) {
  auto res = lhs;
  res += rhs;
  return res;
}

/**
 * @relates Region
 * @brief Subtract a region and a scalar.
 */
template <Index N = 2>
Region<N> operator-(const Region<N>& lhs, Index rhs) {
  auto res = lhs;
  res -= rhs;
  return res;
}

} // namespace Litl

#endif
