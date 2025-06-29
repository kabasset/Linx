// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_MIXINS_MATH_H
#define LINX_BASE_MIXINS_MATH_H

#include "Linx/Base/Functional.h"
#include "Linx/Base/Types.h"
#include "Linx/Base/concepts/Arithmetic.h"

#include <Kokkos_Core.hpp>
#include <cmath>

namespace Linx {

/**
 * @ingroup pixelwise
 * @ingroup mixins
 * @brief Mixin to provide mathematical operations and transforms to a container.
 * 
 * Implements element-wise mathematical functions which may take a range or scalar argument (or none).
 * In the former case, the number of elements in the range must match that of the container.
 * @see pixelwise
 * @see https://en.cppreference.com/w/cpp/header/cmath for functions description
 */
template <typename T, typename TDerived>
struct MathFunctionsMixin {
#define LINX_MATH_UNARY_INPLACE(func) \
  /** @brief Apply `std::##func##()` in place. */ \
  const TDerived& func() const \
  { \
    return LINX_CRTP_CONST_DERIVED.transform(#func, KOKKOS_LAMBDA(const T& e) { return std::func(e); }); \
  }

#define LINX_MATH_BINARY_INPLACE(func) \
  /** @brief Apply `std::##func##()` in place. */ \
  template <NotConvertibleTo<T> TRhs> \
  const TDerived& func(const TRhs& rhs) const \
  { \
    return LINX_CRTP_CONST_DERIVED \
        .transform(#func, KOKKOS_LAMBDA(const T& e, const T& f) { return std::func(e, f); }, rhs); \
  }

#define LINX_MATH_BINARY_SCALAR_INPLACE(func) \
  /** @brief Apply `std::##func##()` in place. */ \
  template <std::convertible_to<T> TRhs> \
  const TDerived& func(const TRhs& rhs) const \
  { \
    return LINX_CRTP_CONST_DERIVED.transform(#func, KOKKOS_LAMBDA(const T& e) { return std::func(e, rhs); }); \
  }

  LINX_MATH_UNARY_INPLACE(abs)
  LINX_MATH_BINARY_INPLACE(max)
  LINX_MATH_BINARY_SCALAR_INPLACE(max)
  LINX_MATH_BINARY_INPLACE(min)
  LINX_MATH_BINARY_SCALAR_INPLACE(min)
  LINX_MATH_BINARY_INPLACE(fdim)
  LINX_MATH_BINARY_SCALAR_INPLACE(fdim)
  LINX_MATH_UNARY_INPLACE(ceil)
  LINX_MATH_UNARY_INPLACE(floor)
  LINX_MATH_BINARY_INPLACE(fmod)
  LINX_MATH_BINARY_SCALAR_INPLACE(fmod)
  LINX_MATH_UNARY_INPLACE(trunc)
  LINX_MATH_UNARY_INPLACE(round)

  LINX_MATH_UNARY_INPLACE(cos)
  LINX_MATH_UNARY_INPLACE(sin)
  LINX_MATH_UNARY_INPLACE(tan)
  LINX_MATH_UNARY_INPLACE(acos)
  LINX_MATH_UNARY_INPLACE(asin)
  LINX_MATH_UNARY_INPLACE(atan)
  LINX_MATH_BINARY_INPLACE(atan2)
  LINX_MATH_BINARY_SCALAR_INPLACE(atan2)
  LINX_MATH_UNARY_INPLACE(cosh)
  LINX_MATH_UNARY_INPLACE(sinh)
  LINX_MATH_UNARY_INPLACE(tanh)
  LINX_MATH_UNARY_INPLACE(acosh)
  LINX_MATH_UNARY_INPLACE(asinh)
  LINX_MATH_UNARY_INPLACE(atanh)

  LINX_MATH_UNARY_INPLACE(exp)
  LINX_MATH_UNARY_INPLACE(exp2)
  LINX_MATH_UNARY_INPLACE(expm1)
  LINX_MATH_UNARY_INPLACE(log)
  LINX_MATH_UNARY_INPLACE(log2)
  LINX_MATH_UNARY_INPLACE(log10)
  LINX_MATH_UNARY_INPLACE(logb)
  LINX_MATH_UNARY_INPLACE(ilogb)
  LINX_MATH_UNARY_INPLACE(log1p)
  LINX_MATH_BINARY_INPLACE(pow)
  LINX_MATH_BINARY_SCALAR_INPLACE(pow)
  LINX_MATH_UNARY_INPLACE(sqrt)
  LINX_MATH_UNARY_INPLACE(cbrt)
  LINX_MATH_BINARY_INPLACE(hypot)
  LINX_MATH_BINARY_SCALAR_INPLACE(hypot)

  LINX_MATH_UNARY_INPLACE(erf)
  LINX_MATH_UNARY_INPLACE(erfc)
  LINX_MATH_UNARY_INPLACE(tgamma)
  LINX_MATH_UNARY_INPLACE(lgamma)

#undef LINX_MATH_UNARY_INPLACE
#undef LINX_MATH_BINARY_INPLACE
};

#define LINX_MATH_UNARY_NEWINSTANCE(func) \
  /** @relatesalso MathFunctionsMixin @brief Apply `std::##func##()` (new instance). */ \
  template <typename T, typename TDerived> \
  TDerived func(const MathFunctionsMixin<T, TDerived>& in) \
  { \
    const auto& derived = static_cast<const TDerived&>(in); \
    auto out = derived.copy_as(compose_label(#func, derived)); \
    out.func(); \
    return out; \
  }

#define LINX_MATH_BINARY_NEWINSTANCE(func) \
  /** @relatesalso MathFunctionsMixin @brief Apply `std::##func##()` (new instance). */ \
  template <typename T, typename TDerived, typename TRhs> \
  TDerived func(const MathFunctionsMixin<T, TDerived>& lhs, const TRhs& rhs) \
  { \
    const auto& derived = static_cast<const TDerived&>(lhs); \
    auto out = derived.copy_as(compose_label(#func, derived, rhs)); \
    out.func(rhs); \
    return out; \
  }

LINX_MATH_UNARY_NEWINSTANCE(abs)
LINX_MATH_BINARY_NEWINSTANCE(max)
LINX_MATH_BINARY_NEWINSTANCE(min)
LINX_MATH_BINARY_NEWINSTANCE(fdim)
LINX_MATH_UNARY_NEWINSTANCE(ceil)
LINX_MATH_UNARY_NEWINSTANCE(floor)
LINX_MATH_BINARY_NEWINSTANCE(fmod)
LINX_MATH_UNARY_NEWINSTANCE(trunc)
LINX_MATH_UNARY_NEWINSTANCE(round)

LINX_MATH_UNARY_NEWINSTANCE(cos)
LINX_MATH_UNARY_NEWINSTANCE(sin)
LINX_MATH_UNARY_NEWINSTANCE(tan)
LINX_MATH_UNARY_NEWINSTANCE(acos)
LINX_MATH_UNARY_NEWINSTANCE(asin)
LINX_MATH_UNARY_NEWINSTANCE(atan)
LINX_MATH_BINARY_NEWINSTANCE(atan2)
LINX_MATH_UNARY_NEWINSTANCE(cosh)
LINX_MATH_UNARY_NEWINSTANCE(sinh)
LINX_MATH_UNARY_NEWINSTANCE(tanh)
LINX_MATH_UNARY_NEWINSTANCE(acosh)
LINX_MATH_UNARY_NEWINSTANCE(asinh)
LINX_MATH_UNARY_NEWINSTANCE(atanh)

LINX_MATH_UNARY_NEWINSTANCE(exp)
LINX_MATH_UNARY_NEWINSTANCE(exp2)
LINX_MATH_UNARY_NEWINSTANCE(expm1)
LINX_MATH_UNARY_NEWINSTANCE(log)
LINX_MATH_UNARY_NEWINSTANCE(log2)
LINX_MATH_UNARY_NEWINSTANCE(log10)
LINX_MATH_UNARY_NEWINSTANCE(logb)
LINX_MATH_UNARY_NEWINSTANCE(ilogb)
LINX_MATH_UNARY_NEWINSTANCE(log1p)
LINX_MATH_BINARY_NEWINSTANCE(pow)
LINX_MATH_UNARY_NEWINSTANCE(sqrt)
LINX_MATH_UNARY_NEWINSTANCE(cbrt)
LINX_MATH_BINARY_NEWINSTANCE(hypot)

LINX_MATH_UNARY_NEWINSTANCE(erf)
LINX_MATH_UNARY_NEWINSTANCE(erfc)
LINX_MATH_UNARY_NEWINSTANCE(tgamma)
LINX_MATH_UNARY_NEWINSTANCE(lgamma)

struct Sqrt {
  KOKKOS_INLINE_FUNCTION auto operator()(const auto& e) const
  {
    return Kokkos::sqrt(e);
  }
};

struct Cbrt {
  KOKKOS_INLINE_FUNCTION auto operator()(const auto& e) const
  {
    return Kokkos::cbrt(e);
  }
};

#undef LINX_MATH_UNARY_NEWINSTANCE
#undef LINX_MATH_BINARY_NEWINSTANCE

} // namespace Linx

#endif
