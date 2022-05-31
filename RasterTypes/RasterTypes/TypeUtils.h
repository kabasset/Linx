// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERTYPES_TYPEUTILS_H
#define _RASTERTYPES_TYPEUTILS_H

#include <complex>
#include <limits>

namespace Cnes {

/**
 * @brief Helper class to make complex numbers from real numbers.
 * @details
 * Does nothing for real numbers.
 */
template <typename T>
struct Complexifier {

  /**
   * @brief The type for real numbers, the component type for complex numbers.
   */
  using Component = T;

  /**
   * @brief Make some complex with same real and imaginary parts.
   */
  static inline T complexify(T in) {
    return in;
  }

  /**
   * @brief Apply some function twice, to get the real and imaginary parts.
   */
  template <typename TFunc, typename TArg>
  static inline T apply(TFunc&& func, TArg&& arg) {
    return std::forward<TFunc>(func)(std::forward<TArg>(arg));
  }
};

/// @cond

template <typename T>
struct Complexifier<std::complex<T>> {

  using Component = T;

  static inline std::complex<T> complexify(T in) {
    return {in, in};
  }

  template <typename TFunc, typename TArg>
  static inline std::complex<T> apply(TFunc&& func, TArg&& arg) {
    return {std::forward<TFunc>(func)(std::forward<TArg>(arg)), std::forward<TFunc>(func)(std::forward<TArg>(arg))};
  }
};

/// @endcond

/**
 * @brief Numeric limits and related key values of a value type.
 */
template <typename T>
struct Limits {

  /**
   * @brief 0 in general, or `false` for Booleans.
   */
  static T zero() {
    return Complexifier<T>::complexify(0);
  }

  /**
   * @brief 1 in general, or `true` for Booleans, or 1 + i for complexes.
   */
  static T one() {
    return Complexifier<T>::complexify(1);
  }

  /**
   * @brief The lowest possible value.
   */
  static T min() {
    return Complexifier<T>::complexify(std::numeric_limits<T>::lowest());
  }

  /**
   * @brief The highest possible value.
   */
  static T max() {
    return Complexifier<T>::complexify(std::numeric_limits<T>::max());
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
   * @brief The max over two in general, rounded up for integers, or `true` for Booleans.
   */
  static T halfMax() {
    return max() / 2 + std::is_integral<T>::value;
  }
};

} // namespace Cnes

#endif // _RASTERTYPES_TYPEUTILS_H
