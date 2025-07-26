// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

namespace Linx {

namespace Impl {

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
  using Lhs = Vector<TLhs>;
  using Rhs = Vector<TRhs>;
  using T = decltype(typename Lhs::element_type() + typename Rhs::element_type());
  if constexpr (Lhs::static_empty_flag) {
    return +rhs;
  } else if constexpr (Rhs::static_empty_flag) {
    return +lhs;
  } else if constexpr (Lhs::static_coefs_flag && Rhs::static_coefs_flag) {
    return Impl::static_add_impl<T>(lhs, rhs, std::make_index_sequence<std::max(TLhs::size(), TRhs::size())>());
  } else if constexpr (Lhs::static_size_flag && Rhs::static_size_flag) {
    constexpr auto N = std::max(Lhs::n, Rhs::n);
    auto out = Vector<T[N]>();
    for (std::size_t i = 0; i < N; ++i) {
      out[i] = lhs.get_or(i, 0) + rhs.get_or(i, 0);
    }
    return out;
  } else {
    auto size = std::max<std::size_t>(lhs.size(), rhs.size());
    auto out = Vector<T*>(Forward(), size);
    for (std::size_t i = 0; i < size; ++i) {
      out[i] = lhs.get_or(i, 0) + rhs.get_or(i, 0);
    }
    return out;
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
  using Lhs = Vector<TLhs>;
  using Rhs = Vector<TRhs>;
  using T = decltype(typename Lhs::element_type() - typename Rhs::element_type());
  if constexpr (Lhs::static_empty_flag) {
    return -rhs;
  } else if constexpr (Rhs::static_empty_flag) {
    return +lhs;
  } else if constexpr (Lhs::static_coefs_flag && Rhs::static_coefs_flag) {
    return Impl::static_subtract_impl<T>(lhs, rhs, std::make_index_sequence<std::max(TLhs::size(), TRhs::size())>());
  } else if constexpr (Lhs::static_size_flag && Rhs::static_size_flag) {
    constexpr auto N = std::max(Lhs::n, Rhs::n);
    auto out = Vector<T[N]>();
    for (std::size_t i = 0; i < N; ++i) {
      out[i] = lhs.get_or(i, 0) - rhs.get_or(i, 0);
    }
    return out;
  } else {
    auto size = std::max<std::size_t>(lhs.size(), rhs.size());
    auto out = Vector<T*>(Forward(), size);
    for (std::size_t i = 0; i < size; ++i) {
      out[i] = lhs.get_or(i, 0) - rhs.get_or(i, 0);
    }
    return out;
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

} // namespace Linx
