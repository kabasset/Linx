// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLBASE_MATH_H
#define _LITLBASE_MATH_H

#include "LitlBase/SeqUtils.h" // isIterable

#include <algorithm>
#include <cmath>

namespace Litl {

/**
 * @brief π
 */
template <typename T>
T pi() {
  static const T out = std::acos(T(-1));
  return out;
}

/// @cond
namespace Internal {

template <Index P>
struct AbspowImpl {
  template <typename T>
  static T pow(T x) {
    return x * x * AbspowImpl<P - 2>::pow(x);
  }
};

template <>
struct AbspowImpl<0> {
  template <typename T>
  static T pow(T x) {
    return bool(x);
  }
};

template <>
struct AbspowImpl<1> {
  template <typename T>
  static T pow(T x) {
    return std::abs(x);
  }
};

template <>
struct AbspowImpl<2> {
  template <typename T>
  static T pow(T x) {
    return x * x;
  }
};

} // namespace Internal
/// @endcond

/**
 * @brief Compute the absolute value of an integral power.
 */
template <Index P, typename T>
T abspow(T x) {
  return Internal::AbspowImpl<P>::pow(x);
}

/**
 * @ingroup pixelwise
 * @ingroup mixins
 * @brief Mixin to provide mathematical operations and transforms to a container.
 * 
 * Implements element-wise mathematical functions which may take an iterable or scalar argument (or none).
 * In the former case, the number of elements in the iterable must match that of the container.
 * @see pixelwise
 * @see https://en.cppreference.com/w/cpp/header/cmath for functions description
 */
template <typename T, typename TDerived>
struct MathFunctionsMixin {

#define LITL_MATH_UNARY_INPLACE(function) \
  /** @brief Apply std::##function##(). */ \
  TDerived& function() { \
    auto* derived = static_cast<TDerived*>(this); \
    std::transform(derived->begin(), derived->end(), derived->begin(), [](auto e) { \
      return std::function(e); \
    }); \
    return *derived; \
  }

#define LITL_MATH_BINARY_INPLACE(function) \
  /** @brief Apply std::##function##(). */ \
  template <typename U> \
  const std::enable_if_t<isIterable<U>::value, TDerived>& function(const U& other) { \
    auto* derived = static_cast<TDerived*>(this); \
    std::transform(derived->begin(), derived->end(), other.begin(), derived->begin(), [](auto e, auto f) { \
      return std::function(e, f); \
    }); \
    return *derived; \
  }

#define LITL_MATH_BINARY_SCALAR_INPLACE(function) \
  /** @brief Apply std::##function##(). */ \
  template <typename U> \
  std::enable_if_t<not isIterable<U>::value, TDerived>& function(U other) { \
    auto* derived = static_cast<TDerived*>(this); \
    std::transform(derived->begin(), derived->end(), derived->begin(), [=](auto e) { \
      return std::function(e, other); \
    }); \
    return *derived; \
  } // TODO rm enable_if and merge with previous function thanks to if constexpr

  LITL_MATH_UNARY_INPLACE(abs)
  LITL_MATH_BINARY_INPLACE(max)
  LITL_MATH_BINARY_SCALAR_INPLACE(max)
  LITL_MATH_BINARY_INPLACE(min)
  LITL_MATH_BINARY_SCALAR_INPLACE(min)
  LITL_MATH_BINARY_INPLACE(fdim)
  LITL_MATH_BINARY_SCALAR_INPLACE(fdim)
  LITL_MATH_UNARY_INPLACE(ceil)
  LITL_MATH_UNARY_INPLACE(floor)
  LITL_MATH_BINARY_INPLACE(fmod)
  LITL_MATH_BINARY_SCALAR_INPLACE(fmod)
  LITL_MATH_UNARY_INPLACE(trunc)
  LITL_MATH_UNARY_INPLACE(round)

  LITL_MATH_UNARY_INPLACE(cos)
  LITL_MATH_UNARY_INPLACE(sin)
  LITL_MATH_UNARY_INPLACE(tan)
  LITL_MATH_UNARY_INPLACE(acos)
  LITL_MATH_UNARY_INPLACE(asin)
  LITL_MATH_UNARY_INPLACE(atan)
  LITL_MATH_BINARY_INPLACE(atan2)
  LITL_MATH_BINARY_SCALAR_INPLACE(atan2)
  LITL_MATH_UNARY_INPLACE(cosh)
  LITL_MATH_UNARY_INPLACE(sinh)
  LITL_MATH_UNARY_INPLACE(tanh)
  LITL_MATH_UNARY_INPLACE(acosh)
  LITL_MATH_UNARY_INPLACE(asinh)
  LITL_MATH_UNARY_INPLACE(atanh)

  LITL_MATH_UNARY_INPLACE(exp)
  LITL_MATH_UNARY_INPLACE(exp2)
  LITL_MATH_UNARY_INPLACE(expm1)
  LITL_MATH_UNARY_INPLACE(log)
  LITL_MATH_UNARY_INPLACE(log2)
  LITL_MATH_UNARY_INPLACE(log10)
  LITL_MATH_UNARY_INPLACE(logb)
  LITL_MATH_UNARY_INPLACE(ilogb)
  LITL_MATH_UNARY_INPLACE(log1p)
  LITL_MATH_BINARY_INPLACE(pow)
  LITL_MATH_BINARY_SCALAR_INPLACE(pow)
  LITL_MATH_UNARY_INPLACE(sqrt)
  LITL_MATH_UNARY_INPLACE(cbrt)
  LITL_MATH_BINARY_INPLACE(hypot)
  LITL_MATH_BINARY_SCALAR_INPLACE(hypot)

  LITL_MATH_UNARY_INPLACE(erf)
  LITL_MATH_UNARY_INPLACE(erfc)
  LITL_MATH_UNARY_INPLACE(tgamma)
  LITL_MATH_UNARY_INPLACE(lgamma)

#undef LITL_MATH_UNARY_INPLACE
#undef LITL_MATH_BINARY_INPLACE
};

#define LITL_MATH_UNARY_NEWINSTANCE(function) \
  /** @relates MathFunctionsMixin @brief Apply std::##function##() (new instance). */ \
  template <typename T, typename TDerived> \
  TDerived function(const MathFunctionsMixin<T, TDerived>& in) { \
    TDerived out(static_cast<const TDerived&>(in)); \
    out.function(); \
    return out; \
  }

#define LITL_MATH_BINARY_NEWINSTANCE(function) \
  /** @relates MathFunctionsMixin @brief Apply std::##function##() (new instance). */ \
  template <typename T, typename TDerived, typename TOther> \
  TDerived function(const MathFunctionsMixin<T, TDerived>& in, const TOther& other) { \
    TDerived out(static_cast<const TDerived&>(in)); \
    out.function(other); \
    return out; \
  }

LITL_MATH_UNARY_NEWINSTANCE(abs)
LITL_MATH_BINARY_NEWINSTANCE(max)
LITL_MATH_BINARY_NEWINSTANCE(min)
LITL_MATH_BINARY_NEWINSTANCE(fdim)
LITL_MATH_UNARY_NEWINSTANCE(ceil)
LITL_MATH_UNARY_NEWINSTANCE(floor)
LITL_MATH_BINARY_NEWINSTANCE(fmod)
LITL_MATH_UNARY_NEWINSTANCE(trunc)
LITL_MATH_UNARY_NEWINSTANCE(round)

LITL_MATH_UNARY_NEWINSTANCE(cos)
LITL_MATH_UNARY_NEWINSTANCE(sin)
LITL_MATH_UNARY_NEWINSTANCE(tan)
LITL_MATH_UNARY_NEWINSTANCE(acos)
LITL_MATH_UNARY_NEWINSTANCE(asin)
LITL_MATH_UNARY_NEWINSTANCE(atan)
LITL_MATH_BINARY_NEWINSTANCE(atan2)
LITL_MATH_UNARY_NEWINSTANCE(cosh)
LITL_MATH_UNARY_NEWINSTANCE(sinh)
LITL_MATH_UNARY_NEWINSTANCE(tanh)
LITL_MATH_UNARY_NEWINSTANCE(acosh)
LITL_MATH_UNARY_NEWINSTANCE(asinh)
LITL_MATH_UNARY_NEWINSTANCE(atanh)

LITL_MATH_UNARY_NEWINSTANCE(exp)
LITL_MATH_UNARY_NEWINSTANCE(exp2)
LITL_MATH_UNARY_NEWINSTANCE(expm1)
LITL_MATH_UNARY_NEWINSTANCE(log)
LITL_MATH_UNARY_NEWINSTANCE(log2)
LITL_MATH_UNARY_NEWINSTANCE(log10)
LITL_MATH_UNARY_NEWINSTANCE(logb)
LITL_MATH_UNARY_NEWINSTANCE(ilogb)
LITL_MATH_UNARY_NEWINSTANCE(log1p)
LITL_MATH_BINARY_NEWINSTANCE(pow)
LITL_MATH_UNARY_NEWINSTANCE(sqrt)
LITL_MATH_UNARY_NEWINSTANCE(cbrt)
LITL_MATH_BINARY_NEWINSTANCE(hypot)

LITL_MATH_UNARY_NEWINSTANCE(erf)
LITL_MATH_UNARY_NEWINSTANCE(erfc)
LITL_MATH_UNARY_NEWINSTANCE(tgamma)
LITL_MATH_UNARY_NEWINSTANCE(lgamma)

#undef LITL_MATH_UNARY_NEWINSTANCE
#undef LITL_MATH_BINARY_NEWINSTANCE

} // namespace Litl

#endif
