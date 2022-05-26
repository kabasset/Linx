// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_MATHFUNCTIONS_H
#define _RASTER_MATHFUNCTIONS_H

#include "RasterTypes/SeqUtils.h" // isIterable

#include <algorithm>
#include <cmath>

namespace Cnes {

/**
 * @brief 3.14
 */
template <typename T>
T pi() {
  static const T out = std::acos(T(-1));
  return out;
}

/**
 * @ingroup mixins
 * @brief Mixin to provide mathematical operations and transforms to a container.
 * @details
 * Implements pixel-wise mathematical functions which may take an iterable or scalar argument (or none).
 * In the former case, the number of elements in the iterable must match that of the container.
 * @see https://en.cppreference.com/w/cpp/header/cmath for functions description
 */
template <typename T, typename TDerived>
struct MathFunctionsMixin {

#define CNES_RASTER_MATH_UNARY_INPLACE(function) \
  TDerived& function() { \
    auto* derived = static_cast<TDerived*>(this); \
    std::transform(derived->begin(), derived->end(), derived->begin(), [](auto e) { \
      return std::function(e); \
    }); \
    return *derived; \
  }

#define CNES_RASTER_MATH_BINARY_INPLACE(function) \
  template <typename U> \
  const std::enable_if_t<isIterable<U>::value, TDerived>& function(const U& other) { \
    auto* derived = static_cast<TDerived*>(this); \
    std::transform(derived->begin(), derived->end(), other.begin(), derived->begin(), [](auto e, auto f) { \
      return std::function(e, f); \
    }); \
    return *derived; \
  } \
\
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

  CNES_RASTER_MATH_UNARY_INPLACE(abs) ///< Apply `std::abs()`
  CNES_RASTER_MATH_BINARY_INPLACE(max) ///< Apply `std::max()`
  CNES_RASTER_MATH_BINARY_INPLACE(min) ///< Apply `std::min()`
  CNES_RASTER_MATH_BINARY_INPLACE(fdim) ///< Apply `std::fdim()`
  CNES_RASTER_MATH_UNARY_INPLACE(ceil) ///< Apply `std::ceil()`
  CNES_RASTER_MATH_UNARY_INPLACE(floor) ///< Apply `std::floor()`
  CNES_RASTER_MATH_BINARY_INPLACE(fmod) ///< Apply `std::fmod()`
  CNES_RASTER_MATH_UNARY_INPLACE(trunc) ///< Apply `std::trunc()`
  CNES_RASTER_MATH_UNARY_INPLACE(round) ///< Apply `std::round()`

  CNES_RASTER_MATH_UNARY_INPLACE(cos) ///< Apply `std::cos()`
  CNES_RASTER_MATH_UNARY_INPLACE(sin) ///< Apply `std::sin()`
  CNES_RASTER_MATH_UNARY_INPLACE(tan) ///< Apply `std::tan()`
  CNES_RASTER_MATH_UNARY_INPLACE(acos) ///< Apply `std::acos()`
  CNES_RASTER_MATH_UNARY_INPLACE(asin) ///< Apply `std::asin()`
  CNES_RASTER_MATH_UNARY_INPLACE(atan) ///< Apply `std::atan()`
  CNES_RASTER_MATH_BINARY_INPLACE(atan2) ///< Apply `std::atan2()`
  CNES_RASTER_MATH_UNARY_INPLACE(cosh) ///< Apply `std::cosh()`
  CNES_RASTER_MATH_UNARY_INPLACE(sinh) ///< Apply `std::sinh()`
  CNES_RASTER_MATH_UNARY_INPLACE(tanh) ///< Apply `std::tanh()`
  CNES_RASTER_MATH_UNARY_INPLACE(acosh) ///< Apply `std::acosh()`
  CNES_RASTER_MATH_UNARY_INPLACE(asinh) ///< Apply `std::asinh()`
  CNES_RASTER_MATH_UNARY_INPLACE(atanh) ///< Apply `std::atanh()`

  CNES_RASTER_MATH_UNARY_INPLACE(exp) ///< Apply `std::exp()`
  CNES_RASTER_MATH_UNARY_INPLACE(exp2) ///< Apply `std::exp2()`
  CNES_RASTER_MATH_UNARY_INPLACE(expm1) ///< Apply `std::expm1()`
  CNES_RASTER_MATH_UNARY_INPLACE(log) ///< Apply `std::log()`
  CNES_RASTER_MATH_UNARY_INPLACE(log2) ///< Apply `std::log2()`
  CNES_RASTER_MATH_UNARY_INPLACE(log10) ///< Apply `std::log10()`
  CNES_RASTER_MATH_UNARY_INPLACE(logb) ///< Apply `std::logb()`
  CNES_RASTER_MATH_UNARY_INPLACE(ilogb) ///< Apply `std::ilogb()`
  CNES_RASTER_MATH_UNARY_INPLACE(log1p) ///< Apply `std::log1p()`
  CNES_RASTER_MATH_BINARY_INPLACE(pow) ///< Apply `std::pow()`
  CNES_RASTER_MATH_UNARY_INPLACE(sqrt) ///< Apply `std::sqrt()`
  CNES_RASTER_MATH_UNARY_INPLACE(cbrt) ///< Apply `std::cbrt()`
  CNES_RASTER_MATH_BINARY_INPLACE(hypot) ///< Apply `std::hypot()`

  CNES_RASTER_MATH_UNARY_INPLACE(erf) ///< Apply `std::erf()`
  CNES_RASTER_MATH_UNARY_INPLACE(erfc) ///< Apply `std::erfc()`
  CNES_RASTER_MATH_UNARY_INPLACE(tgamma) ///< Apply `std::tgamma()`
  CNES_RASTER_MATH_UNARY_INPLACE(lgamma) ///< Apply `std::lgamma()`

  /// @}

#undef CNES_RASTER_MATH_UNARY_INPLACE
#undef CNES_RASTER_MATH_BINARY_INPLACE
};

#define CNES_RASTER_MATH_UNARY_NEWINSTANCE(function) \
  template <typename T, typename TDerived> \
  TDerived function(const MathFunctionsMixin<T, TDerived>& in) { \
    TDerived out(static_cast<const TDerived&>(in)); \
    out.function(); \
    return out; \
  }

#define CNES_RASTER_MATH_BINARY_NEWINSTANCE(function) \
  template <typename T, typename TDerived, typename TOther> \
  TDerived function(const MathFunctionsMixin<T, TDerived>& in, const TOther& other) { \
    TDerived out(static_cast<const TDerived&>(in)); \
    out.function(other); \
    return out; \
  }

CNES_RASTER_MATH_UNARY_NEWINSTANCE(abs) ///< Apply `std::abs()`
CNES_RASTER_MATH_BINARY_NEWINSTANCE(max) ///< Apply `std::max()`
CNES_RASTER_MATH_BINARY_NEWINSTANCE(min) ///< Apply `std::min()`
CNES_RASTER_MATH_BINARY_NEWINSTANCE(fdim) ///< Apply `std::fdim()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(ceil) ///< Apply `std::ceil()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(floor) ///< Apply `std::floor()`
CNES_RASTER_MATH_BINARY_NEWINSTANCE(fmod) ///< Apply `std::fmod()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(trunc) ///< Apply `std::trunc()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(round) ///< Apply `std::round()`

CNES_RASTER_MATH_UNARY_NEWINSTANCE(cos) ///< Apply `std::cos()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(sin) ///< Apply `std::sin()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(tan) ///< Apply `std::tan()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(acos) ///< Apply `std::acos()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(asin) ///< Apply `std::asin()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(atan) ///< Apply `std::atan()`
CNES_RASTER_MATH_BINARY_NEWINSTANCE(atan2) ///< Apply `std::atan2()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(cosh) ///< Apply `std::cosh()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(sinh) ///< Apply `std::sinh()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(tanh) ///< Apply `std::tanh()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(acosh) ///< Apply `std::acosh()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(asinh) ///< Apply `std::asinh()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(atanh) ///< Apply `std::atanh()`

CNES_RASTER_MATH_UNARY_NEWINSTANCE(exp) ///< Apply `std::exp()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(exp2) ///< Apply `std::exp2()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(expm1) ///< Apply `std::expm1()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(log) ///< Apply `std::log()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(log2) ///< Apply `std::log2()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(log10) ///< Apply `std::log10()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(logb) ///< Apply `std::logb()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(ilogb) ///< Apply `std::ilogb()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(log1p) ///< Apply `std::log1p()`
CNES_RASTER_MATH_BINARY_NEWINSTANCE(pow) ///< Apply `std::pow()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(sqrt) ///< Apply `std::sqrt()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(cbrt) ///< Apply `std::cbrt()`
CNES_RASTER_MATH_BINARY_NEWINSTANCE(hypot) ///< Apply `std::hypot()`

CNES_RASTER_MATH_UNARY_NEWINSTANCE(erf) ///< Apply `std::erf()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(erfc) ///< Apply `std::erfc()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(tgamma) ///< Apply `std::tgamma()`
CNES_RASTER_MATH_UNARY_NEWINSTANCE(lgamma) ///< Apply `std::lgamma()`

#undef CNES_RASTER_MATH_UNARY_NEWINSTANCE
#undef CNES_RASTER_MATH_BINARY_NEWINSTANCE

} // namespace Cnes

#endif // _RASTER_MATHFUNCTIONS_H
