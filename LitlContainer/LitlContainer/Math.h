// Copyright (C) 2022, Antoine Basset
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLCONTAINER_MATH_H
#define _LITLCONTAINER_MATH_H

#include "RasterTypes/SeqUtils.h" // isIterable

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

/**
 * @ingroup pixelwise
 * @ingroup mixins
 * @brief Mixin to provide mathematical operations and transforms to a container.
 * @details
 * Implements element-wise mathematical functions which may take an iterable or scalar argument (or none).
 * In the former case, the number of elements in the iterable must match that of the container.
 * @see pixelwise
 * @see https://en.cppreference.com/w/cpp/header/cmath for functions description
 */
template <typename T, typename TDerived>
struct MathFunctionsMixin {

#define LITL_MATH_UNARY_INPLACE(function) \
  TDerived& function() { \
    auto* derived = static_cast<TDerived*>(this); \
    std::transform(derived->begin(), derived->end(), derived->begin(), [](auto e) { \
      return std::function(e); \
    }); \
    return *derived; \
  }

#define LITL_MATH_BINARY_INPLACE(function) \
  template <typename U> \
  const std::enable_if_t<isIterable<U>::value, TDerived>& function(const U& other) { \
    auto* derived = static_cast<TDerived*>(this); \
    std::transform(derived->begin(), derived->end(), other.begin(), derived->begin(), [](auto e, auto f) { \
      return std::function(e, f); \
    }); \
    return *derived; \
  }

#define LITL_MATH_BINARY_SCALAR_INPLACE(function) \
  template <typename U> \
  std::enable_if_t<not isIterable<U>::value, TDerived>& function(U other) { \
    auto* derived = static_cast<TDerived*>(this); \
    std::transform(derived->begin(), derived->end(), derived->begin(), [=](auto e) { \
      return std::function(e, other); \
    }); \
    return *derived; \
  }

  /// @{
  /// @group_modifiers

  LITL_MATH_UNARY_INPLACE(abs) ///< Apply `std::abs()`
  LITL_MATH_BINARY_INPLACE(max) ///< Apply `std::max()`
  LITL_MATH_BINARY_SCALAR_INPLACE(max) ///< Apply `std::max()`
  LITL_MATH_BINARY_INPLACE(min) ///< Apply `std::min()`
  LITL_MATH_BINARY_SCALAR_INPLACE(min) ///< Apply `std::min()`
  LITL_MATH_BINARY_INPLACE(fdim) ///< Apply `std::fdim()`
  LITL_MATH_BINARY_SCALAR_INPLACE(fdim) ///< Apply `std::fdim()`
  LITL_MATH_UNARY_INPLACE(ceil) ///< Apply `std::ceil()`
  LITL_MATH_UNARY_INPLACE(floor) ///< Apply `std::floor()`
  LITL_MATH_BINARY_INPLACE(fmod) ///< Apply `std::fmod()`
  LITL_MATH_BINARY_SCALAR_INPLACE(fmod) ///< Apply `std::fmod()`
  LITL_MATH_UNARY_INPLACE(trunc) ///< Apply `std::trunc()`
  LITL_MATH_UNARY_INPLACE(round) ///< Apply `std::round()`

  LITL_MATH_UNARY_INPLACE(cos) ///< Apply `std::cos()`
  LITL_MATH_UNARY_INPLACE(sin) ///< Apply `std::sin()`
  LITL_MATH_UNARY_INPLACE(tan) ///< Apply `std::tan()`
  LITL_MATH_UNARY_INPLACE(acos) ///< Apply `std::acos()`
  LITL_MATH_UNARY_INPLACE(asin) ///< Apply `std::asin()`
  LITL_MATH_UNARY_INPLACE(atan) ///< Apply `std::atan()`
  LITL_MATH_BINARY_INPLACE(atan2) ///< Apply `std::atan2()`
  LITL_MATH_BINARY_SCALAR_INPLACE(atan2) ///< Apply `std::atan2()`
  LITL_MATH_UNARY_INPLACE(cosh) ///< Apply `std::cosh()`
  LITL_MATH_UNARY_INPLACE(sinh) ///< Apply `std::sinh()`
  LITL_MATH_UNARY_INPLACE(tanh) ///< Apply `std::tanh()`
  LITL_MATH_UNARY_INPLACE(acosh) ///< Apply `std::acosh()`
  LITL_MATH_UNARY_INPLACE(asinh) ///< Apply `std::asinh()`
  LITL_MATH_UNARY_INPLACE(atanh) ///< Apply `std::atanh()`

  LITL_MATH_UNARY_INPLACE(exp) ///< Apply `std::exp()`
  LITL_MATH_UNARY_INPLACE(exp2) ///< Apply `std::exp2()`
  LITL_MATH_UNARY_INPLACE(expm1) ///< Apply `std::expm1()`
  LITL_MATH_UNARY_INPLACE(log) ///< Apply `std::log()`
  LITL_MATH_UNARY_INPLACE(log2) ///< Apply `std::log2()`
  LITL_MATH_UNARY_INPLACE(log10) ///< Apply `std::log10()`
  LITL_MATH_UNARY_INPLACE(logb) ///< Apply `std::logb()`
  LITL_MATH_UNARY_INPLACE(ilogb) ///< Apply `std::ilogb()`
  LITL_MATH_UNARY_INPLACE(log1p) ///< Apply `std::log1p()`
  LITL_MATH_BINARY_INPLACE(pow) ///< Apply `std::pow()`
  LITL_MATH_BINARY_SCALAR_INPLACE(pow) ///< Apply `std::pow()`
  LITL_MATH_UNARY_INPLACE(sqrt) ///< Apply `std::sqrt()`
  LITL_MATH_UNARY_INPLACE(cbrt) ///< Apply `std::cbrt()`
  LITL_MATH_BINARY_INPLACE(hypot) ///< Apply `std::hypot()`
  LITL_MATH_BINARY_SCALAR_INPLACE(hypot) ///< Apply `std::hypot()`

  LITL_MATH_UNARY_INPLACE(erf) ///< Apply `std::erf()`
  LITL_MATH_UNARY_INPLACE(erfc) ///< Apply `std::erfc()`
  LITL_MATH_UNARY_INPLACE(tgamma) ///< Apply `std::tgamma()`
  LITL_MATH_UNARY_INPLACE(lgamma) ///< Apply `std::lgamma()`

  /// @}

#undef LITL_MATH_UNARY_INPLACE
#undef LITL_MATH_BINARY_INPLACE
};

#define LITL_MATH_UNARY_NEWINSTANCE(function) \
  template <typename T, typename TDerived> \
  TDerived function(const MathFunctionsMixin<T, TDerived>& in) { \
    TDerived out(static_cast<const TDerived&>(in)); \
    out.function(); \
    return out; \
  }

#define LITL_MATH_BINARY_NEWINSTANCE(function) \
  template <typename T, typename TDerived, typename TOther> \
  TDerived function(const MathFunctionsMixin<T, TDerived>& in, const TOther& other) { \
    TDerived out(static_cast<const TDerived&>(in)); \
    out.function(other); \
    return out; \
  }

LITL_MATH_UNARY_NEWINSTANCE(abs) ///< Apply `std::abs()` @ingroup pixelwise
LITL_MATH_BINARY_NEWINSTANCE(max) ///< Apply `std::max()` @ingroup pixelwise
LITL_MATH_BINARY_NEWINSTANCE(min) ///< Apply `std::min()` @ingroup pixelwise
LITL_MATH_BINARY_NEWINSTANCE(fdim) ///< Apply `std::fdim()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(ceil) ///< Apply `std::ceil()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(floor) ///< Apply `std::floor()` @ingroup pixelwise
LITL_MATH_BINARY_NEWINSTANCE(fmod) ///< Apply `std::fmod()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(trunc) ///< Apply `std::trunc()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(round) ///< Apply `std::round()` @ingroup pixelwise

LITL_MATH_UNARY_NEWINSTANCE(cos) ///< Apply `std::cos()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(sin) ///< Apply `std::sin()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(tan) ///< Apply `std::tan()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(acos) ///< Apply `std::acos()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(asin) ///< Apply `std::asin()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(atan) ///< Apply `std::atan()` @ingroup pixelwise
LITL_MATH_BINARY_NEWINSTANCE(atan2) ///< Apply `std::atan2()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(cosh) ///< Apply `std::cosh()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(sinh) ///< Apply `std::sinh()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(tanh) ///< Apply `std::tanh()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(acosh) ///< Apply `std::acosh()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(asinh) ///< Apply `std::asinh()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(atanh) ///< Apply `std::atanh()` @ingroup pixelwise

LITL_MATH_UNARY_NEWINSTANCE(exp) ///< Apply `std::exp()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(exp2) ///< Apply `std::exp2()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(expm1) ///< Apply `std::expm1()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(log) ///< Apply `std::log()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(log2) ///< Apply `std::log2()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(log10) ///< Apply `std::log10()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(logb) ///< Apply `std::logb()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(ilogb) ///< Apply `std::ilogb()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(log1p) ///< Apply `std::log1p()` @ingroup pixelwise
LITL_MATH_BINARY_NEWINSTANCE(pow) ///< Apply `std::pow()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(sqrt) ///< Apply `std::sqrt()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(cbrt) ///< Apply `std::cbrt()` @ingroup pixelwise
LITL_MATH_BINARY_NEWINSTANCE(hypot) ///< Apply `std::hypot()` @ingroup pixelwise

LITL_MATH_UNARY_NEWINSTANCE(erf) ///< Apply `std::erf()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(erfc) ///< Apply `std::erfc()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(tgamma) ///< Apply `std::tgamma()` @ingroup pixelwise
LITL_MATH_UNARY_NEWINSTANCE(lgamma) ///< Apply `std::lgamma()` @ingroup pixelwise

#undef LITL_MATH_UNARY_NEWINSTANCE
#undef LITL_MATH_BINARY_NEWINSTANCE

} // namespace Litl

#endif
