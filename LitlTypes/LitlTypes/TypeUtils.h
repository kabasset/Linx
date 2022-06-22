// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTYPES_TYPEUTILS_H
#define _LITLTYPES_TYPEUTILS_H

#include <complex>
#include <limits>
#include <utility> // forward

namespace Litl {

/**
 * @brief Define a default virtual destructor.
 */
#define LITL_VIRTUAL_DTOR(classname) \
  /** @brief Destructor. */ \
  virtual ~classname() = default;

/**
 * @brief Define default copy constructor and assignment operator.
 */
#define LITL_DEFAULT_COPYABLE(classname) \
  /** @brief Copy constructor. */ \
  classname(const classname&) = default; \
  /** @brief Copy assignment operator. */ \
  classname& operator=(const classname&) = default;

/**
 * @brief Define deleted copy constructor and assignment operator.
 */
#define LITL_NON_COPYABLE(classname) \
  /** @brief Deleted copy constructor. */ \
  classname(const classname&) = delete; \
  /** @brief Deleted copy assignment operator. */ \
  classname& operator=(const classname&) = delete;

/**
 * @brief Define default move constructor and assignment operator.
 */
#define LITL_DEFAULT_MOVABLE(classname) \
  /** @brief Move constructor. */ \
  classname(classname&&) = default; \
  /** @brief Move assignment operator. */ \
  classname& operator=(classname&&) = default;

/**
 * @brief Define deleted move constructor and assignment operator.
 */
#define LITL_NON_MOVABLE(classname) \
  /** @brief Deleted move constructor. */ \
  classname(classname&&) = delete; \
  /** @brief Deleted move assignment operator. */ \
  classname& operator=(classname&&) = delete;

/**
 * @brief The signed integer type which represents indices.
 */
using Index = long;

/**
 * @brief Type traits.
 */
template <typename T>
struct TypeTraits {
  /**
   * @brief The floating point type which corresponds to `T`.
   */
  using Floating = std::conditional_t<std::is_floating_point<T>::value, T, double>;

  /**
   * @brief The scalar type which corresponds to `T`.
   */
  using Scalar = T;
};

/// @cond
template <typename T>
struct TypeTraits<std::complex<T>> {
  using Floating = std::complex<T>;
  using Scalar = T;
};
/// @endcond

/**
 * @brief Clamp some input value between a min and max values.
 */
template <typename T, typename U>
inline T clamp(T in, U min, U max) {
  return in < min ? min : in > max ? max : in;
}

/**
 * @brief Compute the floor of an input floating point value, as an integer value.
 */
template <typename TInt, typename TFloat>
TInt floor(TFloat in) {
  TInt out = in;
  return out - (in < 0);
}

/**
 * @brief Utility type for SFINAE, equivalent to C++17's `std::void_t`.
 */
template <typename...>
using templateVoid = void;

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
   * @brief The type for real numbers, the component type for complex numbers.
   */
  using Component = typename Complexifier<T>::Component;

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
    return Complexifier<T>::complexify(std::numeric_limits<Component>::lowest());
  }

  /**
   * @brief The highest possible value.
   */
  static T max() {
    return Complexifier<T>::complexify(std::numeric_limits<Component>::max());
  }

  /**
   * @brief The infinity value if defined, or `max()` otherwise.
   */
  static T inf() {
    constexpr auto infinity = std::numeric_limits<Component>::infinity();
    return infinity ? Complexifier<T>::complexify(infinity) : max();
  }

  /**
   * @brief The difference between two consecutive values.
   */
  static T epsilon() {
    return Complexifier<T>::complexify(std::numeric_limits<Component>::epsilon());
  }

  /**
   * @brief The min plus one epsilon.
   */
  static T almostMin() {
    return min() + epsilon();
  }

  /**
   * @brief The max minus one epsilon.
   */
  static T almostMax() {
    return max() - epsilon();
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

} // namespace Litl

#endif
