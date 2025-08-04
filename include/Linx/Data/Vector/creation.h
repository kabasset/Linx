// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_VECTOR_CREATION_H
#define LINX_DATA_VECTOR_CREATION_H

namespace Linx {

/**
 * @brief Create a dynamic-size vector.
 */
template <typename T>
constexpr auto vec(std::initializer_list<T> coefs)
{
  return Vector<T*>(coefs.begin(), coefs.end());
}

/**
 * @brief Create a static-size vector.
 */
template <typename T, std::size_t N>
constexpr auto vec(std::array<T, N> coefs)
{
  return Vector<T[N]>(Forward(), LINX_MOVE(coefs));
}

/**
 * @brief Create a static-size vector.
 */
template <typename T0, std::convertible_to<T0>... Ts>
constexpr auto vec(T0 coef0, Ts... coefs)
{
  return vec(std::array {coef0, T0 {coefs}...});
}

/**
 * @brief Create a static-coefficients vector.
 */
template <std::integral auto Coef0, std::integral auto... Coefs>
static constexpr auto vec()
{
  using T = decltype(Coef0);
  return Vector<std::integer_sequence<T, Coef0, Coefs...>>();
}

namespace Impl {

/**
 * @brief Helper function to silent warning.
 */
static constexpr auto discard_first(auto, auto out)
{
  return out;
}

/**
 * @brief Helper function to repeat a coefficient.
 */
template <std::size_t... Is>
constexpr auto vec_impl(auto coef, std::index_sequence<Is...>)
{
  return vec(discard_first(Is, coef)...);
}

/**
 * @brief Helper function to repeat a coefficient.
 */
template <std::integral auto Coef, typename T, auto... Is>
static constexpr auto vec_impl(std::integer_sequence<T, Is...>)
{
  return vec<discard_first(Is, Coef)...>();
}

} // namespace Impl

/**
 * @brief Create a dynamic-size vector of given dimension full of a given value.
 * @tparam D The dimension
 * @param value The value
 */
constexpr auto vec(Dimension d, auto value)
{
  using T = decltype(value);
  return Vector<T*>(Forward(), d.value, value);
}

/**
 * @brief Create a static-size vector of given dimension full of a given value.
 * @tparam D The dimension
 * @param value The value
 */
template <Dimension D>
constexpr auto vec(auto value)
{
  return Impl::vec_impl(value, std::make_index_sequence<D.value>());
}

/**
 * @brief Create a static-coefficients vector of given dimension full of a given value.
 * @tparam D The dimension
 * @tparam Value The value
 */
template <Dimension D, std::integral auto Value = 0>
static constexpr auto vec()
{
  using T = decltype(Value);
  return Impl::vec_impl<Value>(std::make_integer_sequence<T, D.value>());
}

/**
 * @brief Vector 0.
 */
static constexpr auto vec()
{
  return Vector<>();
}

} // namespace Linx

#endif
