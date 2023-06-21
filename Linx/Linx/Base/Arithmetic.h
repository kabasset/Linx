// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXBASE_ARITHMETIC_H
#define _LINXBASE_ARITHMETIC_H

#include <algorithm>
#include <boost/operators.hpp>
#include <functional>
#include <type_traits>

namespace Linx {

#define LINX_VECTOR_OPERATOR_INPLACE(op) \
  TDerived& operator op##=(const TDerived& rhs) { \
    std::transform( \
        static_cast<TDerived*>(this)->begin(), \
        static_cast<TDerived*>(this)->end(), \
        rhs.begin(), \
        static_cast<TDerived*>(this)->begin(), \
        [](auto e, auto f) { \
          return e op f; \
        }); \
    return static_cast<TDerived&>(*this); \
  }

#define LINX_SCALAR_OPERATOR_INPLACE(op) \
  TDerived& operator op##=(const T& rhs) { \
    std::transform( \
        static_cast<TDerived*>(this)->begin(), \
        static_cast<TDerived*>(this)->end(), \
        static_cast<TDerived*>(this)->begin(), \
        [&](auto e) { \
          return e op rhs; \
        }); \
    return static_cast<TDerived&>(*this); \
  }

/**
 * @ingroup concepts
 * @requirements{VectorArithmetic}
 * @brief Vector space arithmetic requirements.
 * 
 * Implements vector space arithmetic operators
 * (uppercase letters are for vectors, lowercase letters are for scalars):
 * - Vector-additive: V += U, W = V + U, V -= U, W = V - U;
 * - Scalar-additive: V += a, V = U + a, V = a + U, V -= a, V = U + a, V = a - U, V++, ++V, V--, --V;
 * - Scalar-multiplicative: V *= a, V = U * a, V = a * U, V /= a, V = U / a.
 */
class VectorArithmetic;

/**
 * @ingroup concepts
 * @requirements{EuclidArithmetic}
 * @brief Euclidean ring arithmetic requirements.
 * 
 * Adds the following operators to `VectorArithmetic`:
 * - Vector-multiplicative: V *= U, W = U * V, V /= U, W = V / U;
 * - Scalar-modable: V %= a, V = U % a;
 * - Vector-modable: V %= U, W = V % U;
 */
class EuclidArithmetic;

/**
 * @ingroup pixelwise
 * @ingroup mixins
 * @brief Mixin to provide arithmetics operators to a container.
 * @tparam TSpecs The operators specifications, can be `void`
 * @tparam T The contained element value type
 * @tparam TDerived The container which inherits this class
 * 
 * @tspecialization{VectorArithmetic}
 * @tspecialization{EuclidArithmetic}
 */
template <typename TSpecs, typename T, typename TDerived>
struct ArithmeticMixin {}; // Empy base class for void

/// @cond

/**
 * @ingroup pixelwise
 * @ingroup mixins
 * @brief `VectorArithmetic` specialization.
 * @satisfies{VectorArithmetic}
 */
template <typename T, typename TDerived>
struct ArithmeticMixin<VectorArithmetic, T, TDerived> :
    boost::additive<TDerived>,
    boost::additive<TDerived, T>,
    boost::subtractable2_left<TDerived, T>,
    boost::unit_steppable<TDerived>,
    boost::multiplicative<TDerived, T>,
    boost::modable<TDerived>,
    boost::modable<TDerived, T> {

  /// @{
  /// @group_modifiers

  LINX_VECTOR_OPERATOR_INPLACE(+) ///< V += U
  LINX_SCALAR_OPERATOR_INPLACE(+) ///< V += a

  LINX_VECTOR_OPERATOR_INPLACE(-) ///< V -= U
  LINX_SCALAR_OPERATOR_INPLACE(-) ///< V -= a

  LINX_SCALAR_OPERATOR_INPLACE(*) ///< V *= a

  LINX_SCALAR_OPERATOR_INPLACE(/) ///< V /= a

  LINX_VECTOR_OPERATOR_INPLACE(%) ///< V %= U
  LINX_SCALAR_OPERATOR_INPLACE(%) ///< V %= a

  /**
   * @brief ++V
   */
  TDerived& operator++() {
    std::transform(
        static_cast<TDerived*>(this)->begin(),
        static_cast<TDerived*>(this)->end(),
        static_cast<TDerived*>(this)->begin(),
        [](auto rhs) {
          return ++rhs;
        });
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief --V
   */
  TDerived& operator--() {
    std::transform(
        static_cast<TDerived*>(this)->begin(),
        static_cast<TDerived*>(this)->end(),
        static_cast<TDerived*>(this)->begin(),
        [](auto rhs) {
          return --rhs;
        });
    return static_cast<TDerived&>(*this);
  }

  /// @group_operations

  /**
   * @brief Copy.
   */
  TDerived operator+() const {
    return *this;
  }

  /**
   * @brief Compute the opposite.
   */
  TDerived operator-() const {
    TDerived res = static_cast<const TDerived&>(*this);
    std::transform(res.begin(), res.end(), res.begin(), [&](auto r) {
      return -r;
    });
    return res;
  }

  /// @}
};

/**
 * @ingroup pixelwise
 * @ingroup mixins
 * @brief `EuclidArithmetic` specialization.
 * @satisfies{EuclidArithmetic}
 */
template <typename T, typename TDerived>
struct ArithmeticMixin<EuclidArithmetic, T, TDerived> :
    boost::additive<TDerived>,
    boost::additive<TDerived, T>,
    boost::subtractable2_left<TDerived, T>,
    boost::unit_steppable<TDerived>,
    boost::multiplicative<TDerived>,
    boost::multiplicative<TDerived, T>,
    boost::modable<TDerived>,
    boost::modable<TDerived, T> {

  /// @{
  /// @group_modifiers

  LINX_VECTOR_OPERATOR_INPLACE(+) ///< V += U
  LINX_SCALAR_OPERATOR_INPLACE(+) ///< V += a

  LINX_VECTOR_OPERATOR_INPLACE(-) ///< V -= U
  LINX_SCALAR_OPERATOR_INPLACE(-) ///< V -= a

  LINX_VECTOR_OPERATOR_INPLACE(*) ///< V *= U
  LINX_SCALAR_OPERATOR_INPLACE(*) ///< V *= a

  LINX_VECTOR_OPERATOR_INPLACE(/) ///< V /= U
  LINX_SCALAR_OPERATOR_INPLACE(/) ///< V /= a

  LINX_VECTOR_OPERATOR_INPLACE(%) ///< V %= U
  LINX_SCALAR_OPERATOR_INPLACE(%) ///< V %= a

  /**
   * @brief ++V
   */
  TDerived& operator++() {
    std::transform(
        static_cast<TDerived*>(this)->begin(),
        static_cast<TDerived*>(this)->end(),
        static_cast<TDerived*>(this)->begin(),
        [](auto rhs) {
          return ++rhs;
        });
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief --V
   */
  TDerived& operator--() {
    std::transform(
        static_cast<TDerived*>(this)->begin(),
        static_cast<TDerived*>(this)->end(),
        static_cast<TDerived*>(this)->begin(),
        [](auto rhs) {
          return --rhs;
        });
    return static_cast<TDerived&>(*this);
  }

  /// @group_operations

  /**
   * @brief Copy.
   */
  TDerived operator+() const {
    return *this;
  }

  /**
   * @brief Compute the opposite.
   */
  TDerived operator-() const {
    TDerived res = static_cast<const TDerived&>(*this);
    std::transform(res.begin(), res.end(), res.begin(), [&](auto r) {
      return -r;
    });
    return res;
  }

  /// @}
};

/// @endcond

#undef LINX_VECTOR_OPERATOR_INPLACE
#undef LINX_SCALAR_OPERATOR_INPLACE

} // namespace Linx

#endif
