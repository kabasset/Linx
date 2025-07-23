// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

namespace Linx {

namespace Impl {

template <typename T>
struct VectorTraits {
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

} // namespace Impl

template <typename TLhs, typename TRhs>
auto operator+(const Vector<TLhs>& lhs, const Vector<TRhs>& rhs)
{
  using LTraits = Impl::VectorTraits<TLhs>;
  using RTraits = Impl::VectorTraits<TRhs>;
  using T = decltype(typename LTraits::element_type() + typename RTraits::element_type());
  if constexpr (LTraits::empty) {
    return rhs;
  } else if constexpr (RTraits::empty) {
    return lhs;
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
    auto out = Vector<T*>(size);
    for (std::size_t i = 0; i < size; ++i) {
      out[i] = lhs.get_or(i, 0) + rhs.get_or(i, 0);
    }
    return out;
  }
}

} // namespace Linx
