// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

namespace Linx {

namespace Impl {

template <typename T>
struct VectorTraits { // FIXME directly to Vector
  static constexpr auto n = Vector<T>::n;
  static constexpr bool has_static_rank = (n >= 0);
  static constexpr bool has_static_coefs = std::is_same_v<typename Vector<T>::Container, void>;
  static constexpr bool empty = (n == 0);
  using element_type = typename Vector<T>::element_type;
};

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
  using LTraits = Impl::VectorTraits<TLhs>;
  using RTraits = Impl::VectorTraits<TRhs>;
  using T = decltype(typename LTraits::element_type() + typename RTraits::element_type());
  if constexpr (LTraits::empty) {
    return +rhs;
  } else if constexpr (RTraits::empty) {
    return +lhs;
  } else if constexpr (LTraits::has_static_coefs && RTraits::has_static_coefs) {
    return Impl::static_add_impl<T>(lhs, rhs, std::make_index_sequence<std::max(TLhs::size(), TRhs::size())>());
  } else if constexpr (LTraits::has_static_rank && RTraits::has_static_rank) {
    constexpr auto N = std::max(LTraits::n, RTraits::n);
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
  if constexpr (Impl::VectorTraits<TLhs>::has_static_coefs) {
    return lhs + vec<Dimension {Vector<TLhs>::n}>(rhs);
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
  if constexpr (Impl::VectorTraits<T>::empty) {
    return in;
  } else if constexpr (Impl::VectorTraits<T>::has_static_coefs) {
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
  using LTraits = Impl::VectorTraits<TLhs>;
  using RTraits = Impl::VectorTraits<TRhs>;
  using T = decltype(typename LTraits::element_type() - typename RTraits::element_type());
  if constexpr (LTraits::empty) {
    return -rhs;
  } else if constexpr (RTraits::empty) {
    return +lhs;
  } else if constexpr (LTraits::has_static_coefs && RTraits::has_static_coefs) {
    return Impl::static_subtract_impl<T>(lhs, rhs, std::make_index_sequence<std::max(TLhs::size(), TRhs::size())>());
  } else if constexpr (LTraits::has_static_rank && RTraits::has_static_rank) {
    constexpr auto N = std::max(LTraits::n, RTraits::n);
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
  if constexpr (Impl::VectorTraits<TLhs>::has_static_coefs) {
    return lhs - vec<Dimension {Vector<TLhs>::n}>(rhs);
  } else {
    auto out = +lhs;
    for (std::size_t i = 0; i < out.size(); ++i) {
      out[i] -= rhs;
    }
    return out;
  }
}

} // namespace Linx
