// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_MIXINS_ARITHMETIC_H
#define LINX_BASE_MIXINS_ARITHMETIC_H

#include "Linx/Base/Functional.h"
#include "Linx/Base/Types.h" // LINX_FORWARD

#include <Kokkos_Core.hpp>
#include <functional>
#include <type_traits>

namespace Linx {

#define LINX_SCALAR_OPERATOR_INPLACE(op, Func) \
  /** @brief Apply operator `op`. */ \
  const TDerived& operator op(const T & rhs) const \
  { \
    return LINX_CRTP_CONST_DERIVED.apply(compose_label(#op, LINX_CRTP_CONST_DERIVED, rhs), Func(rhs)); \
  }

#define LINX_SCALAR_OPERATOR_NEWINSTANCE(op, op_in) \
  /** @brief Apply operator `op` (new instance). */ \
  friend TDerived operator op(const TDerived& lhs, const T& rhs) \
  { \
    TDerived out = lhs.copy_as(compose_label(#op, lhs, rhs)); \
    out op_in rhs; \
    return out; \
  }

#define LINX_SCALAR_OPERATOR(op_in, op_new, Func) \
  LINX_SCALAR_OPERATOR_INPLACE(op_in, Func) \
  LINX_SCALAR_OPERATOR_NEWINSTANCE(op_new, op_in)

#define LINX_VECTOR_OPERATOR_INPLACE(op, Func) \
  /** @brief Apply operator `op`. */ \
  template <typename U, typename UDerived> \
  const TDerived& operator op(const CopyArithmeticMixin<U, UDerived>& rhs) const \
  { \
    const auto& derived_rhs = static_cast<const UDerived&>(rhs); \
    return LINX_CRTP_CONST_DERIVED \
        .apply(compose_label(#op, LINX_CRTP_CONST_DERIVED, derived_rhs), Func(), derived_rhs); \
  }

#define LINX_VECTOR_OPERATOR_NEWINSTANCE(op, op_in) \
  /** @brief Apply operator `op` (new instance). */ \
  template <typename U, typename UDerived> \
  friend TDerived operator op(const TDerived& lhs, const CopyArithmeticMixin<U, UDerived>& rhs) \
  { \
    const auto& derived_rhs = static_cast<const UDerived&>(rhs); \
    TDerived out = lhs.copy_as(compose_label(#op, lhs, derived_rhs)); \
    out op_in derived_rhs; \
    return out; \
  }

#define LINX_VECTOR_OPERATOR(op_in, op_new, Func) \
  LINX_VECTOR_OPERATOR_INPLACE(op_in, Func) \
  LINX_VECTOR_OPERATOR_NEWINSTANCE(op_new, op_in)

#define LINX_OPERATOR(op_in, op_new, Func) \
  LINX_SCALAR_OPERATOR(op_in, op_new, Func) \
  LINX_VECTOR_OPERATOR(op_in, op_new, Func)

/**
 * @ingroup pixelwise
 * @brief Mixin which provides arithmetic operators to a derived container class.
 * @tparam TDerived The container which inherits this class
 * 
 * @see `BooleanArithmeticMixin`
 * @see `VectorArithmeticMixin`
 * @see `EuclidArithmeticMixin`
 */
template <typename T, typename TDerived>
struct CopyArithmeticMixin {
  /**
   * @brief Deep copy with a new label.
   */
  TDerived copy_as(const std::string& label) const
  {
    auto out = same_layout(label, LINX_CRTP_CONST_DERIVED);
    Kokkos::deep_copy(out.container(), LINX_CRTP_CONST_DERIVED.container());
    return out;
  }

  /**
   * @brief Deep copy.
   */
  TDerived operator+() const
  {
    return copy_as(compose_label("copy", LINX_CRTP_CONST_DERIVED));
  }
};

/**
 * @ingroup pixelwise
 * @brief @copybrief CopyArithmeticMixin
 * 
 * Implements boolean arithmetic operators
 * (uppercase letters are for arrays, lowercase letters are for scalars):
 * - `B = not A`;
 * - `A |= b`, `C = A || b`;
 * - `A |= B`, `C = A || B`;
 * - `A &= b`, `C = A && b`;
 * - `A &= B`, `C = A && B`.
 * 
 * @see `VectorArithmeticMixin`
 * @see `EuclidArithmeticMixin`
 */
template <typename T, typename TDerived>
struct BooleanArithmeticMixin : public CopyArithmeticMixin<T, TDerived> {
  /**
   * @brief Apply operator `not`.
   */
  TDerived operator not() const
  {
    TDerived res = copy_as(compose_label("not", LINX_CRTP_CONST_DERIVED));
    res.apply("!", Not());
    return res;
  }

  LINX_OPERATOR(|=, ||, Or)
  LINX_OPERATOR(&=, &&, And)
};

/**
 * @ingroup pixelwise
 * @brief @copybrief CopyArithmeticMixin
 * 
 * Implements vector space arithmetic operators
 * (uppercase letters are for arrays, lowercase letters are for scalars):
 * - `++A`, `B = A++`;
 * - `A += b`, `C = A + b`;
 * - `A += B`, `C = A + B`;
 * - `B = -A`;
 * - `--A`, `B = A--`;
 * - `A -= b`, `C = A - b`;
 * - `A -= B`, `C = A - B`;
 * - `A *= b`, `C = A * b`;
 * - `A /= b`, `C = A / b`;
 * - `A %= b`, `C = A % b`.
 * 
 * @see `BooleanArithmeticMixin`
 * @see `EuclidArithmeticMixin`
 */
template <typename T, typename TDerived>
struct VectorArithmeticMixin : public CopyArithmeticMixin<T, TDerived> {
  /**
   * @brief Apply operator `-`.
   */
  TDerived operator-() const
  {
    TDerived res = copy_as(compose_label("negate", LINX_CRTP_CONST_DERIVED));
    res.apply("-", Negate());
    return res;
  }

  LINX_OPERATOR(+=, +, Add)

  LINX_OPERATOR(-=, -, Subtract)

  LINX_SCALAR_OPERATOR(*=, *, Multiply)

  LINX_SCALAR_OPERATOR(/=, /, Divide)

  LINX_SCALAR_OPERATOR(%=, %, Modulus)

  /**
   * @brief Apply operator `++`.
   */
  const TDerived& operator++() const
  {
    return LINX_CRTP_CONST_DERIVED.apply("++", Add(1));
  }

  /**
   * @brief Apply operator `++`.
   */
  TDerived operator++(int) const
  {
    auto out = +LINX_CRTP_CONST_DERIVED;
    ++out;
    return out;
  }

  /**
   * @brief Apply operator `--`.
   */
  const TDerived& operator--() const
  {
    return LINX_CRTP_CONST_DERIVED.apply("--", Subtract(1));
  }

  /**
   * @brief Apply operator `--`.
   */
  TDerived operator--(int) const
  {
    auto out = +LINX_CRTP_CONST_DERIVED;
    --out;
    return out;
  }
};

/**
 * @ingroup pixelwise
 * @brief @copybrief CopyArithmeticMixin
 * 
 * Implements Euclidean ring arithmetic operators
 * (uppercase letters are for arrays, lowercase letters are for scalars):
 * - `++A`, `B = A++`;
 * - `A += b`, `C = A + b`;
 * - `A += B`, `C = A + B`;
 * - `B = -A`;
 * - `--A`, `B = A--`;
 * - `A -= b`, `C = A - b`;
 * - `A -= B`, `C = A - B`;
 * - `A *= b`, `C = A * b`;
 * - `A *= B`, `C = A * B`;
 * - `A /= b`, `C = A / b`;
 * - `A /= B`, `C = A / B`;
 * - `A %= b`, `C = A % b`;
 * - `A %= B`, `C = A % B`.
 * 
 * @see `BooleanArithmeticMixin`
 * @see `VectorArithmeticMixin`
 */
template <typename T, typename TDerived>
struct EuclidArithmeticMixin : public CopyArithmeticMixin<T, TDerived> {
  /**
   * @brief Apply operator `-`.
   */
  TDerived operator-() const
  {
    TDerived res = copy_as(compose_label("negate", LINX_CRTP_CONST_DERIVED));
    res.apply("-", Negate());
    return res;
  }

  LINX_OPERATOR(+=, +, Add)

  LINX_OPERATOR(-=, -, Subtract)

  LINX_OPERATOR(*=, *, Multiply)

  LINX_OPERATOR(/=, /, Divide)

  LINX_OPERATOR(%=, %, Modulus)

  /**
   * @brief Apply operator `++`.
   */
  const TDerived& operator++() const
  {
    return LINX_CRTP_CONST_DERIVED.apply("++", Add(1));
  }

  /**
   * @brief Apply operator `++`.
   */
  TDerived operator++(int) const
  {
    auto out = +LINX_CRTP_CONST_DERIVED;
    ++out;
    return out;
  }

  /**
   * @brief Apply operator `--`.
   */
  const TDerived& operator--() const
  {
    return LINX_CRTP_CONST_DERIVED.apply("--", Subtract(1));
  }

  /**
   * @brief Apply operator `--`.
   */
  TDerived operator--(int) const
  {
    auto out = +LINX_CRTP_CONST_DERIVED;
    --out;
    return out;
  }
};

#undef LINX_SCALAR_OPERATOR_INPLACE
#undef LINX_SCALAR_OPERATOR_NEWINSTANCE
#undef LINX_VECTOR_OPERATOR_INPLACE
#undef LINX_VECTOR_OPERATOR_NEWINSTANCE
#undef LINX_SCALAR_OPERATOR
#undef LINX_VECTOR_OPERATOR
#undef LINX_OPERATOR

} // namespace Linx

#endif
