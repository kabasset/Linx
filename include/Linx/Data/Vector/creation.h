// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

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
constexpr auto vec(const std::array<T, N>& coefs)
{
  return Vector<T[N]>(std::begin(coefs), std::end(coefs));
}

/**
 * @brief Create a static-size vector.
 */
template <std::integral T0, std::integral... Ts>
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
template <typename T, auto... Is>
constexpr auto vec_impl(auto coef, std::integer_sequence<T, Is...>)
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
 * @brief Fill a static-size vector of given rank with a given value.
 * @tparam R The rank
 * @param coef The coefficient
 */
template <Rank R>
constexpr auto vec(auto coef)
{
  using T = decltype(coef);
  return Impl::vec_impl(coef, std::make_integer_sequence<T, R.n>());
}

/**
 * @brief Fill a static-coefficients vector of given rank with a given value.
 * @tparam R The rank
 * @tparam Coef The coefficient
 */
template <Rank R, std::integral auto Coef = 0>
static constexpr auto vec()
{
  using T = decltype(Coef);
  return Impl::vec_impl<Coef>(std::make_integer_sequence<T, R.n>());
}

/**
 * @brief Vector 0.
 */
static constexpr auto vec()
{
  return Vector<>();
}

} // namespace Linx
