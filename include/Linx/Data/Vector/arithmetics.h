// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_VECTOR_ARITHMETICS_H
#define LINX_DATA_VECTOR_ARITHMETICS_H

#include "Linx/Base/Functional.h"
#include "Linx/Data/Vector/creation.h"

namespace Linx {

namespace Impl {

template <typename T>
static constexpr auto identity_element_or(const auto& func, const T& fallback = T {})
{
  if constexpr (requires { identity_element<T>(func); }) {
    return identity_element<T>(func);
  } else {
    return T {};
  }
}

template <typename T, typename TFunc, typename TLhs, typename TRhs, std::size_t... Is>
static constexpr auto
static_transform_vectors_impl(const Vector<TLhs>&, const Vector<TRhs>&, std::index_sequence<Is...>)
{
  constexpr auto identity = identity_element_or<T>(TFunc());
  using TOut =
      std::integer_sequence<T, TFunc()(get_or<Is, identity>(Vector<TLhs>()), get_or<Is, identity>(Vector<TRhs>()))...>;
  return Vector<TOut>();
}

template <typename T, typename TLhs, typename TRhs, std::size_t... Is>
static constexpr auto static_add_impl(const Vector<TLhs>&, const Vector<TRhs>&, std::index_sequence<Is...>)
{
  using TOut = std::integer_sequence<T, (get_or<Is, 0>(Vector<TLhs>()) + get_or<Is, 0>(Vector<TRhs>()))...>;
  return Vector<TOut>();
}

template <typename T, std::size_t... Is>
static constexpr auto static_opposite_impl(const Vector<T>&, std::index_sequence<Is...>)
{
  using TOut = std::integer_sequence<typename T::value_type, -get<Is>(Vector<T>())...>;
  return Vector<TOut>();
}

template <typename T, typename TLhs, typename TRhs, std::size_t... Is>
static constexpr auto static_subtract_impl(const Vector<TLhs>&, const Vector<TRhs>&, std::index_sequence<Is...>)
{
  using TOut = std::integer_sequence<T, (get_or<Is, 0>(Vector<TLhs>()) - get_or<Is, 0>(Vector<TRhs>()))...>;
  return Vector<TOut>();
}

} // namespace Impl

/**
 * @brief Apply a monoid to each element of two vectors.
 * 
 * If the vectors have different sizes, the resulting size is the greatest of both,
 * and the identity element of the monoid is used for padding.
 */
template <typename TFunc, typename TLhs, typename TRhs>
constexpr auto transform_vectors(const Vector<TLhs>& lhs, const Vector<TRhs>& rhs)
{
  using Lhs = Vector<TLhs>;
  using Rhs = Vector<TRhs>;
  using T = decltype(TFunc()(typename Lhs::element_type(), typename Rhs::element_type()));
  constexpr auto identity = Impl::identity_element_or<T>(TFunc());
  if constexpr (Lhs::static_coefs_flag && Rhs::static_coefs_flag) {
    return Impl::static_transform_vectors_impl<T, TFunc>(
        lhs,
        rhs,
        std::make_index_sequence<std::max(TLhs::size(), TRhs::size())>());
  } else if constexpr (Lhs::static_size_flag && Rhs::static_size_flag) {
    constexpr auto n = std::max(Lhs::n, Rhs::n);
    // auto out = Vector<T[n]>(); // nvcc 12.4 internal error
    auto out = std::array<T, n>();
    for (std::size_t i = 0; i < n; ++i) {
      out[i] = TFunc()(lhs.get_or(i, identity), rhs.get_or(i, identity));
    }
    return vec(LINX_MOVE(out));
  } else {
    auto size = std::max<std::size_t>(lhs.size(), rhs.size());
    auto out = Vector<T*>(Forward(), size);
    for (std::size_t i = 0; i < size; ++i) {
      out[i] = TFunc()(lhs.get_or(i, identity), rhs.get_or(i, identity));
    }
    return out;
  }
}

/**
 * @brief Vector copy.
 */
template <typename T>
constexpr auto operator+(Vector<T> in)
{
  return in; // FIXME return Vector<in::element_type>
}

/**
 * @brief Sum two vectors.
 */
template <typename TLhs, typename TRhs>
constexpr auto operator+(const Vector<TLhs>& lhs, const Vector<TRhs>& rhs)
{
  if constexpr (Vector<TLhs>::static_empty_flag) {
    return +rhs;
  } else if constexpr (Vector<TRhs>::static_empty_flag) {
    return +lhs;
  } else {
    return transform_vectors<Add<>>(lhs, rhs);
  }
}

/**
 * @brief Add a scalar to a vector.
 */
template <typename TLhs, std::convertible_to<typename Vector<TLhs>::value_type> TRhs>
constexpr auto operator+(const Vector<TLhs>& lhs, TRhs rhs)
{
  if constexpr (Vector<TLhs>::static_coefs_flag) {
    if constexpr (requires { TRhs::value; }) {
      return lhs + vec<Dimension {Vector<TLhs>::n}, TRhs::value>();
    } else {
      return lhs + vec<Dimension {Vector<TLhs>::n}>(rhs);
    }
  } else {
    auto out = +lhs;
    for (std::size_t i = 0; i < out.size(); ++i) {
      out[i] += rhs;
    }
    return out;
  }
}

/**
 * @brief Opposite of a vector.
 */
template <typename T>
constexpr auto operator-(Vector<T> in)
{
  if constexpr (Vector<T>::static_empty_flag) {
    return in;
  } else if constexpr (Vector<T>::static_coefs_flag) {
    return Impl::static_opposite_impl(in, std::make_index_sequence<T::size()>());
  } else {
    for (std::size_t i = 0; i < in.size(); ++i) {
      in[i] = -in[i];
    }
    return in;
  }
}

/**
 * @brief Subtract two vectors.
 */
template <typename TLhs, typename TRhs>
constexpr auto operator-(const Vector<TLhs>& lhs, const Vector<TRhs>& rhs)
{
  if constexpr (Vector<TLhs>::static_empty_flag) {
    return -rhs;
  } else if constexpr (Vector<TRhs>::static_empty_flag) {
    return +lhs;
  } else {
    return transform_vectors<Subtract<>>(lhs, rhs);
  }
}

/**
 * @brief Subtract a scalar from a vector.
 */
template <typename TLhs, std::convertible_to<typename Vector<TLhs>::value_type> TRhs>
constexpr auto operator-(const Vector<TLhs>& lhs, TRhs rhs)
{
  if constexpr (Vector<TLhs>::static_coefs_flag) {
    if constexpr (requires { TRhs::value; }) {
      return lhs - vec<Dimension {Vector<TLhs>::n}, TRhs::value>();
    } else {
      return lhs - vec<Dimension {Vector<TLhs>::n}>(rhs);
    }
  } else {
    auto out = +lhs;
    for (std::size_t i = 0; i < out.size(); ++i) {
      out[i] -= rhs;
    }
    return out;
  }
}

template <typename TLhs, typename TRhs>
constexpr auto min(const Vector<TLhs>& lhs, const Vector<TRhs>& rhs)
{
  return transform_vectors<Min<>>(lhs, rhs);
}

template <typename TLhs, typename TRhs>
constexpr auto max(const Vector<TLhs>& lhs, const Vector<TRhs>& rhs)
{
  return transform_vectors<Max<>>(lhs, rhs);
}

} // namespace Linx

#endif
