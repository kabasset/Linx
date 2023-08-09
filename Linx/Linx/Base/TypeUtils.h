// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXBASE_TYPEUTILS_H
#define _LINXBASE_TYPEUTILS_H

#include <complex>
#include <limits>
#include <tuple>
#include <utility> // forward

namespace Linx {

/**
 * @brief List of supported integral types.
 */
#define LINX_SUPPORTED_INTS \
  bool, unsigned char, char, signed char, unsigned short, signed short, unsigned int, signed int, unsigned long, \
      signed long, unsigned long long, signed long long

/**
 * @brief List of supported floating point types.
 */
#define LINX_SUPPORTED_FLOATS float, double, long double

/**
 * @brief List of supported complex types.
 */
#define LINX_SUPPORTED_COMPLEXES std::complex<float>, std::complex<double>, std::complex<long double>

/**
 * @brief List of supported types.
 */
#define LINX_SUPPORTED_TYPES LINX_SUPPORTED_INTS, LINX_SUPPORTED_FLOATS, LINX_SUPPORTED_COMPLEXES

/**
 * @brief List of supported types as a tuple.
 */
using LinxSupportedTypesTuple = std::tuple<LINX_SUPPORTED_TYPES>;

/**
 * @brief `BOOST_AUTO_TEST_CASE_TEMPLATE` for each supported type.
 */
#define LINX_TEST_CASE_TEMPLATE(name) BOOST_AUTO_TEST_CASE_TEMPLATE(name, T, LinxSupportedTypesTuple)

/**
 * @brief Define a default virtual destructor.
 */
#define LINX_VIRTUAL_DTOR(classname) \
  /** @brief Destructor. */ \
  virtual ~classname() = default;

/**
 * @brief Define default copy constructor and assignment operator.
 */
#define LINX_DEFAULT_COPYABLE(classname) \
  /** @brief Copy constructor. */ \
  classname(const classname&) = default; \
  /** @brief Copy assignment operator. */ \
  classname& operator=(const classname&) = default;

/**
 * @brief Define deleted copy constructor and assignment operator.
 */
#define LINX_NON_COPYABLE(classname) \
  /** @brief Deleted copy constructor. */ \
  classname(const classname&) = delete; \
  /** @brief Deleted copy assignment operator. */ \
  classname& operator=(const classname&) = delete;

/**
 * @brief Define default move constructor and assignment operator.
 */
#define LINX_DEFAULT_MOVABLE(classname) \
  /** @brief Move constructor. */ \
  classname(classname&&) = default; \
  /** @brief Move assignment operator. */ \
  classname& operator=(classname&&) = default;

/**
 * @brief Define deleted move constructor and assignment operator.
 */
#define LINX_NON_MOVABLE(classname) \
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
   * 
   * A floating point type wide enough to handle any value of type `T`.
   * Can be complex.
   */
  using Floating = std::conditional_t<std::is_floating_point<T>::value, T, double>;

  /**
   * @brief The scalar type which corresponds to `T`.
   * 
   * The type itself in general, or the value type of complex types.
   */
  using Scalar = T;

  /**
   * @brief Make some `T` from a scalar.
   * 
   * Return the value itself if `T` is already scalar,
   * or a complex with same real and imaginary parts if `T` is complex.
   */
  static inline T from_scalar(Scalar in) {
    return in;
  }

  /**
   * @brief Make some `T` from a scalar function.
   * 
   * If `T` is complex, apply the function twice to get the real and imaginary parts.
   */
  template <typename TFunc, typename TArg>
  static inline T apply_scalar(TFunc&& func, TArg&& arg) {
    return std::forward<TFunc>(func)(std::forward<TArg>(arg));
  }
};

/// @cond
template <typename T>
struct TypeTraits<std::complex<T>> {

  using Floating = std::complex<T>;

  using Scalar = T;

  static inline std::complex<T> from_scalar(T in) {
    return {in, in};
  }

  template <typename TFunc, typename TArg>
  static inline std::complex<T> apply_scalar(TFunc&& func, TArg&& arg) {
    return {std::forward<TFunc>(func)(std::forward<TArg>(arg)), std::forward<TFunc>(func)(std::forward<TArg>(arg))};
  }
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
 * @brief Numeric limits and related key values of a value type.
 */
template <typename T>
struct Limits {

  /**
   * @brief The type for real numbers, the component type for complex numbers.
   */
  using Scalar = typename TypeTraits<T>::Scalar;

  /**
   * @brief 0 in general, or `false` for Booleans.
   */
  static T zero() {
    return TypeTraits<T>::from_scalar(0);
  }

  /**
   * @brief 1 in general, or `true` for Booleans, or 1 + i for complexes.
   */
  static T one() {
    return TypeTraits<T>::from_scalar(1);
  }

  /**
   * @brief The lowest possible value.
   */
  static T min() {
    return TypeTraits<T>::from_scalar(std::numeric_limits<Scalar>::lowest());
  }

  /**
   * @brief The highest possible value.
   */
  static T max() {
    return TypeTraits<T>::from_scalar(std::numeric_limits<Scalar>::max());
  }

  /**
   * @brief The infinity value if defined, or `max()` otherwise.
   */
  static T inf() {
    constexpr auto infinity = std::numeric_limits<Scalar>::infinity();
    return infinity ? TypeTraits<T>::from_scalar(infinity) : max();
  }

  /**
   * @brief The difference between two consecutive values.
   */
  static T epsilon() {
    return TypeTraits<T>::from_scalar(std::numeric_limits<Scalar>::epsilon());
  }

  /**
   * @brief The min plus one epsilon.
   */
  static T almost_min() {
    return min() + epsilon();
  }

  /**
   * @brief The max minus one epsilon.
   */
  static T almost_max() {
    return max() - epsilon();
  }

  /**
   * @brief The min over two.
   */
  static T half_min() {
    return min() / 2;
  }

  /**
   * @brief The max over two in general, rounded up for integers, or `true` for Booleans.
   */
  static T half_max() {
    return max() / 2 + std::is_integral<T>::value;
  }
};

} // namespace Linx

#endif
