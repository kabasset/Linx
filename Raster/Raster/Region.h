/// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_REGION_H
#define _RASTER_REGION_H

#include "Raster/Position.h"

namespace Cnes {

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
   * @brief Create an unlimited region.
   * @details
   * Front and back bounds along each axis are respectively 0 and -1.
   */
  static Region<N> whole() {
    return {Position<N>::zero(), Position<N>::max()};
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

/**
 * @relates Region
 * @brief Clamp a position inside a region.
 */
template <typename T, Index N = 2>
Vector<T, N> clamp(const Vector<T, N>& position, const Region<N>& region) {
  Vector<T, N> out(region.size());
  for (std::size_t i = 0; i < out.size(); ++i) {
    out[i] = clamp(position[i], region.front[i], region.back[i]); // FIXME transform
  }
  return out;
}

/**
 * @relates Region
 * @brief Clamp a position inside a shape.
 */
template <typename T, Index N = 2>
Vector<T, N> clamp(const Vector<T, N>& position, const Position<N>& shape) {
  Vector<T, N> out(shape.size());
  std::transform(position.begin(), position.end(), shape.begin(), out.begin(), [](auto p, auto s) {
    return clamp(p, Index(0), s - 1);
  });
  return out;
}

} // namespace Cnes

#endif // _RASTER_REGION_H
