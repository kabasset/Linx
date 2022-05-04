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
 * @ingroup data_concepts
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

  /// @group_modifiers

  CNES_RASTER_MATH_UNARY_INPLACE(abs)
  CNES_RASTER_MATH_BINARY_INPLACE(max)
  CNES_RASTER_MATH_BINARY_INPLACE(min)
  CNES_RASTER_MATH_BINARY_INPLACE(fdim)
  CNES_RASTER_MATH_UNARY_INPLACE(ceil)
  CNES_RASTER_MATH_UNARY_INPLACE(floor)
  CNES_RASTER_MATH_BINARY_INPLACE(fmod)
  CNES_RASTER_MATH_UNARY_INPLACE(trunc)
  CNES_RASTER_MATH_UNARY_INPLACE(round)

  CNES_RASTER_MATH_UNARY_INPLACE(cos)
  CNES_RASTER_MATH_UNARY_INPLACE(sin)
  CNES_RASTER_MATH_UNARY_INPLACE(tan)
  CNES_RASTER_MATH_UNARY_INPLACE(acos)
  CNES_RASTER_MATH_UNARY_INPLACE(asin)
  CNES_RASTER_MATH_UNARY_INPLACE(atan)
  CNES_RASTER_MATH_BINARY_INPLACE(atan2)
  CNES_RASTER_MATH_UNARY_INPLACE(cosh)
  CNES_RASTER_MATH_UNARY_INPLACE(sinh)
  CNES_RASTER_MATH_UNARY_INPLACE(tanh)
  CNES_RASTER_MATH_UNARY_INPLACE(acosh)
  CNES_RASTER_MATH_UNARY_INPLACE(asinh)
  CNES_RASTER_MATH_UNARY_INPLACE(atanh)

  CNES_RASTER_MATH_UNARY_INPLACE(exp)
  CNES_RASTER_MATH_UNARY_INPLACE(exp2)
  CNES_RASTER_MATH_UNARY_INPLACE(expm1)
  CNES_RASTER_MATH_UNARY_INPLACE(log)
  CNES_RASTER_MATH_UNARY_INPLACE(log2)
  CNES_RASTER_MATH_UNARY_INPLACE(log10)
  CNES_RASTER_MATH_UNARY_INPLACE(logb)
  CNES_RASTER_MATH_UNARY_INPLACE(ilogb)
  CNES_RASTER_MATH_UNARY_INPLACE(log1p)
  CNES_RASTER_MATH_BINARY_INPLACE(pow)
  CNES_RASTER_MATH_UNARY_INPLACE(sqrt)
  CNES_RASTER_MATH_UNARY_INPLACE(cbrt)
  CNES_RASTER_MATH_BINARY_INPLACE(hypot)

  CNES_RASTER_MATH_UNARY_INPLACE(erf)
  CNES_RASTER_MATH_UNARY_INPLACE(erfc)
  CNES_RASTER_MATH_UNARY_INPLACE(tgamma)
  CNES_RASTER_MATH_UNARY_INPLACE(lgamma)

  /// @}

#undef CNES_RASTER_MATH_UNARY_INPLACE
#undef CNES_RASTER_MATH_BINARY_INPLACE

}; // namespace Cnes

} // namespace Cnes

#endif // _RASTER_MATHFUNCTIONS_H
