// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERTYPES_TYPEUTILS_H
#define _RASTERTYPES_TYPEUTILS_H

#include <complex>
#include <limits>

namespace Cnes {

/**
 * @brief Numeric limits and related key values of a value type.
 */
template <typename T>
struct Limits {

  /**
   * @brief 0 or `false` for Booleans.
   */
  static T zero() {
    return {};
  }

  /**
   * @brief 1 or `true` for Booleans or 1 + i for complexes.
   */
  static T one() {
    return {1};
  }

  /**
   * @brief The lowest possible value.
   */
  static T min() {
    return std::numeric_limits<T>::lowest();
  }

  /**
   * @brief The highest possible value.
   */
  static T max() {
    return std::numeric_limits<T>::max();
  }

  /**
   * @brief The min plus one epsilon.
   */
  static T almostMin() {
    return min() + std::numeric_limits<T>::epsilon();
  }

  /**
   * @brief The max minus one epsilon.
   */
  static T almostMax() {
    return max() - std::numeric_limits<T>::epsilon();
  }

  /**
   * @brief The min over two.
   */
  static T halfMin() {
    return min() / 2;
  }

  /**
   * @brief The max over two, or `true` for Booleans.
   */
  static T halfMax() {
    return max() / 2;
  }
};

/// @cond

template <>
struct Limits<bool> {

  static bool zero() {
    return false;
  }
  static bool one() {
    return true;
  }

  static bool min() {
    return false;
  }

  static bool max() {
    return true;
  }

  static bool almostMin() {
    return false;
  }

  static bool almostMax() {
    return true;
  }

  static bool halfMin() {
    return false;
  }

  static bool halfMax() {
    return true;
  }
};

template <typename T>
struct Limits<std::complex<T>> {

  static std::complex<T> zero() {
    return {};
  }
  static std::complex<T> one() {
    return {1, 1};
  }

  static std::complex<T> min() {
    return complexify(Limits<T>::min());
  }

  static std::complex<T> max() {
    return complexify(Limits<T>::max());
  }

  static std::complex<T> almostMin() {
    return complexify(Limits<T>::almostMin());
  }

  static std::complex<T> almostMax() {
    return complexify(Limits<T>::almostMax());
  }

  static std::complex<T> halfMin() {
    return complexify(Limits<T>::halfMin());
  }

  static std::complex<T> halfMax() {
    return complexify(Limits<T>::halfMax());
  }

private:
  static std::complex<T> complexify(T value) {
    return {value, value};
  }
};

/// @endcond

} // namespace Cnes

#endif // _RASTERTYPES_TYPEUTILS_H
