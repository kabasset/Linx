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
  const TDerived& operator op(const T & rhs) const \
  { \
    return LINX_CRTP_CONST_DERIVED.apply(compose_label(#op, LINX_CRTP_CONST_DERIVED, rhs), Func(rhs)); \
  }

#define LINX_SCALAR_OPERATOR_NEWINSTANCE(op, op_in) \
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
  template <typename U, typename UDerived> \
  const TDerived& operator op(const CopyArithmeticMixin<U, UDerived>& rhs) const \
  { \
    const auto& derived_rhs = static_cast<const UDerived&>(rhs); \
    return LINX_CRTP_CONST_DERIVED \
        .apply(compose_label(#op, LINX_CRTP_CONST_DERIVED, derived_rhs), Func(), derived_rhs); \
  }

#define LINX_VECTOR_OPERATOR_NEWINSTANCE(op, op_in) \
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
 * @brief Mixin to provide arithmetics operators to a container.
 * @tparam TDerived The container which inherits this class
 */
template <typename T, typename TDerived>
struct CopyArithmeticMixin {
  /**
   * @brief Deep copy with a new label.
   */
  TDerived copy_as(const std::string& label) const
  {
    TDerived out(label, LINX_CRTP_CONST_DERIVED.shape());
    Kokkos::deep_copy(out.container(), LINX_CRTP_CONST_DERIVED.container());
    return out;
  }

  /**
   * @brief Deep copy.
   */
  TDerived operator+() const
  {
    // TODO if container use_count() <= 1, return this to optimize out temporary objects
    return copy_as(compose_label("copy", LINX_CRTP_CONST_DERIVED));
  }
};

/**
 * @ingroup pixelwise
 * @brief Boolean arithmetic mixin.
 * 
 * Implements boolean arithmetic operators:
 * - V |= U, W = V || U, V &= U, W = V && U, V = !U;
 * - V |= a, W = V || a, V &= a, W = V && a.
 */
template <typename T, typename TDerived>
struct BooleanArithmeticMixin : public CopyArithmeticMixin<T, TDerived> {
  /**
   * @brief Compute the opposite.
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
 * @brief Vector space arithmetic mixin.
 * 
 * Implements vector space arithmetic operators
 * (uppercase letters are for vectors, lowercase letters are for scalars):
 * - Vector-additive: V += U, W = V + U, V -= U, W = V - U;
 * - Scalar-additive: V += a, V = U + a, V = a + U, V -= a, V = U + a, V = a - U, V++, ++V, V--, --V;
 * - Scalar-multiplicative: V *= a, V = U * a, V = a * U, V /= a, V = U / a.
 */
template <typename T, typename TDerived>
struct VectorArithmeticMixin : public CopyArithmeticMixin<T, TDerived> {
  /**
   * @brief Compute the opposite.
   */
  TDerived operator-() const
  {
    TDerived res = copy_as(compose_label("negate", LINX_CRTP_CONST_DERIVED));
    res.apply("-", Negate());
    return res;
  }

  /// @{
  /// @group_modifiers

  LINX_OPERATOR(+=, +, Add)

  LINX_OPERATOR(-=, -, Subtract)

  LINX_SCALAR_OPERATOR(*=, *, Multiply)

  LINX_SCALAR_OPERATOR(/=, /, Divide)

  LINX_SCALAR_OPERATOR(%=, %, Modulus)

  /**
   * @brief ++V
   */
  const TDerived& operator++() const
  {
    return LINX_CRTP_CONST_DERIVED.apply("++", Add(1));
  }

  /**
   * @brief --V
   */
  const TDerived& operator--() const
  {
    return LINX_CRTP_CONST_DERIVED.apply("--", Subtract(1));
  }

  /// @}
};

/**
 * @ingroup pixelwise
 * @brief Euclidean ring arithmetic mixin.
 * 
 * Adds the following operators to `VectorArithmeticMixin`:
 * - Vector-multiplicative: V *= U, W = U * V, V /= U, W = V / U;
 * - Scalar-modable: V %= a, V = U % a;
 * - Vector-modable: V %= U, W = V % U;
 */
template <typename T, typename TDerived>
struct EuclidArithmeticMixin : public CopyArithmeticMixin<T, TDerived> {
  /**
   * @brief Compute the opposite.
   */
  TDerived operator-() const
  {
    TDerived res = copy_as(compose_label("negate", LINX_CRTP_CONST_DERIVED));
    res.apply("-", Negate());
    return res;
  }

  /// @{
  /// @group_modifiers

  LINX_OPERATOR(+=, +, Add)

  LINX_OPERATOR(-=, -, Subtract)

  LINX_OPERATOR(*=, *, Multiply)

  LINX_OPERATOR(/=, /, Divide)

  LINX_OPERATOR(%=, %, Modulus)

  /**
   * @brief ++V
   */
  const TDerived& operator++() const
  {
    return LINX_CRTP_CONST_DERIVED.apply("++", Add(1));
  }

  /**
   * @brief --V
   */
  const TDerived& operator--() const
  {
    return LINX_CRTP_CONST_DERIVED.apply("--", Subtract(1));
  }

  /// @}
};

/// @endcond

#undef LINX_SCALAR_OPERATOR_INPLACE
#undef LINX_SCALAR_OPERATOR_NEWINSTANCE
#undef LINX_VECTOR_OPERATOR_INPLACE
#undef LINX_VECTOR_OPERATOR_NEWINSTANCE
#undef LINX_SCALAR_OPERATOR
#undef LINX_VECTOR_OPERATOR
#undef LINX_OPERATOR

} // namespace Linx

#endif
