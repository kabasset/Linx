// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_VECTORARITHMETIC_H
#define _RASTER_VECTORARITHMETIC_H

#include <algorithm>
#include <boost/operators.hpp>
#include <functional>
#include <type_traits>

namespace Cnes {

/**
 * @ingroup concepts
 * @requirements{VectorArithmetic}
 * @brief Vector-space arithmetic requirements.
 * @details
 * Implements vector space arithmetic operators
 * (uppercase letters are for vectors, lowercase letters are for scalars):
 * - Vector-additive: V += U, W = V + U, V += U, W = V - U;
 * - Scalar-additive: V += a, V = U + a, V = a + U, V -= a, V = U + a, V = a - U, V++, ++V, V--, --V;
 * - Scalar-multiplicative: V *= a, V = U * a, V = a * U, V /= a, V = U / a.
 */

/**
 * @ingroup pixelwise
 * @ingroup mixins
 * @brief Mixin to provide vector space arithmetics to a container.
 * @tparam T The contained element value type
 * @tparam TDerived The container which inherits this class
 * @details
 * In addition to vector space arithmetic operators, this mixin provides
 * `generate()` and `apply()` to apply a function to each element.
 * @satisfies{VectorArithmetic}
 */
template <typename T, typename TDerived>
struct VectorArithmeticMixin :
    boost::additive<TDerived>,
    boost::additive<TDerived, T>,
    boost::subtractable2_left<TDerived, T>,
    boost::unit_steppable<TDerived>,
    boost::multiplicative<TDerived, T> {

  /// @{
  /// @group_modifiers

  /**
   * @brief V += U and W = V + U.
   */
  TDerived& operator+=(const TDerived& rhs) {
    std::transform(
        static_cast<TDerived*>(this)->begin(),
        static_cast<TDerived*>(this)->end(),
        rhs.begin(),
        static_cast<TDerived*>(this)->begin(),
        std::plus<>());
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief V += a, V = U + a, V = a + U.
   */
  TDerived& operator+=(const T& rhs) {
    std::transform(
        static_cast<TDerived*>(this)->begin(),
        static_cast<TDerived*>(this)->end(),
        static_cast<TDerived*>(this)->begin(),
        [&](auto lhs) {
          return lhs + rhs;
        });
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief V -= U and W = V - U.
   */
  TDerived& operator-=(const TDerived& rhs) {
    std::transform(
        static_cast<TDerived*>(this)->begin(),
        static_cast<TDerived*>(this)->end(),
        rhs.begin(),
        static_cast<TDerived*>(this)->begin(),
        std::minus<>());
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief V -= a, V = U - a, V = a - U.
   */
  TDerived& operator-=(const T& rhs) {
    std::transform(
        static_cast<TDerived*>(this)->begin(),
        static_cast<TDerived*>(this)->end(),
        static_cast<TDerived*>(this)->begin(),
        [&](auto lhs) {
          return lhs - rhs;
        });
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief ++V and V++.
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
   * @brief --V and V--.
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

  /**
   * @brief V *= a, V = U * a, V = a * U.
   */
  TDerived& operator*=(const T& rhs) {
    std::transform(
        static_cast<TDerived*>(this)->begin(),
        static_cast<TDerived*>(this)->end(),
        static_cast<TDerived*>(this)->begin(),
        [&](auto lhs) {
          return lhs * rhs;
        });
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief V /= a, V = U / a.
   */
  TDerived& operator/=(const T& rhs) {
    std::transform(
        static_cast<TDerived*>(this)->begin(),
        static_cast<TDerived*>(this)->end(),
        static_cast<TDerived*>(this)->begin(),
        [&](auto lhs) {
          return lhs / rhs;
        });
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief Generate values from a function with optional input containers.
   * @param func The generator function, which takes as many inputs as there are arguments
   * @param args The arguments in the form of containers of compatible sizes
   * @details
   * For example, here is how to imlement element-wise square root and multiplication:
   * \code
   * Container a = ...;
   * Container b = ...;
   * Container res(a.size());
   * res.generate([](auto v) { return std::sqrt(v) }, a); // res = sqrt(a)
   * res.generate([](auto v, auto w) { return v * w; }, a, b); // res = a * b
   * \endcode
   */
  template <typename TFunc, typename... TContainers>
  TDerived& generate(TFunc&& func, const TContainers&... args) {
    auto its = std::make_tuple(args.begin()...);
    for (auto& v : static_cast<TDerived&>(*this)) {
      v = iteratorTupleApply(its, func);
    }
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief Apply a function with optional input containers.
   * @param func The function
   * @param args The arguments in the form of containers of compatible sizes
   * @details
   * If there are _n_ arguments, `func` takes _n_+1 parameters,
   * where the first argument is the element of this container.
   * For example, here is how to imlement in-place element-wise square root and multiplication:
   * \code
   * Container a = ...;
   * Container res = ...;
   * res.apply([](auto v) { return std::sqrt(v); }); // res = sqrt(res)
   * res.apply([](auto v, auto w) { return v * w; }, a); // res *= a
   * \endcode
   */
  template <typename TFunc, typename... TContainers>
  TDerived& apply(TFunc&& func, const TContainers&... args) {
    return generate(std::forward<TFunc>(func), static_cast<TDerived&>(*this), args...);
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

} // namespace Cnes

#endif
