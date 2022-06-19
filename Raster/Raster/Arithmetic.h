// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_ARITHMETIC_H
#define _RASTER_ARITHMETIC_H

#include <algorithm>
#include <boost/operators.hpp>
#include <functional>
#include <type_traits>

namespace Cnes {

#define CNES_RASTER_VECTOR_OPERATOR_INPLACE(op) \
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

#define CNES_RASTER_SCALAR_OPERATOR_INPLACE(op) \
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
 * @details
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
 * @details
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
 * @tparam TSpecs The operators specifications
 * @tparam T The contained element value type
 * @tparam TDerived The container which inherits this class
 * @details
 * In addition to vector space arithmetic operators, this mixin provides
 * `generate()` and `apply()` to apply a function to each element.
 */
template <typename TSpecs, typename T, typename TDerived>
struct ArithmeticMixin;

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

  CNES_RASTER_VECTOR_OPERATOR_INPLACE(+) ///< V += U
  CNES_RASTER_SCALAR_OPERATOR_INPLACE(+) ///< V += a

  CNES_RASTER_VECTOR_OPERATOR_INPLACE(-) ///< V -= U
  CNES_RASTER_SCALAR_OPERATOR_INPLACE(-) ///< V -= a

  CNES_RASTER_SCALAR_OPERATOR_INPLACE(*) ///< V *= a

  CNES_RASTER_SCALAR_OPERATOR_INPLACE(/) ///< V /= a

  CNES_RASTER_VECTOR_OPERATOR_INPLACE(%) ///< V %= U
  CNES_RASTER_SCALAR_OPERATOR_INPLACE(%) ///< V %= a

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

  CNES_RASTER_VECTOR_OPERATOR_INPLACE(+) ///< V += U
  CNES_RASTER_SCALAR_OPERATOR_INPLACE(+) ///< V += a

  CNES_RASTER_VECTOR_OPERATOR_INPLACE(-) ///< V -= U
  CNES_RASTER_SCALAR_OPERATOR_INPLACE(-) ///< V -= a

  CNES_RASTER_VECTOR_OPERATOR_INPLACE(*) ///< V *= U
  CNES_RASTER_SCALAR_OPERATOR_INPLACE(*) ///< V *= a

  CNES_RASTER_VECTOR_OPERATOR_INPLACE(/) ///< V /= U
  CNES_RASTER_SCALAR_OPERATOR_INPLACE(/) ///< V /= a

  CNES_RASTER_VECTOR_OPERATOR_INPLACE(%) ///< V %= U
  CNES_RASTER_SCALAR_OPERATOR_INPLACE(%) ///< V %= a

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

#undef CNES_RASTER_VECTOR_OPERATOR_INPLACE
#undef CNES_RASTER_SCALAR_OPERATOR_INPLACE

} // namespace Cnes

#endif
