// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

namespace Linx {

namespace Impl {

template <typename T>
struct VectorTraits {
  static constexpr bool has_static_rank = (Vector<T>::n >= 0);
  static constexpr bool has_static_coefs = std::is_same_v<typename Vector<T>::Container, void>;
  static constexpr bool is_zero = (Vector<T>::n == 0);
};

template <typename TLhs, typename TRhs, std::size_t... Is>
static constexpr auto static_add_impl(const Vector<TLhs>&, const Vector<TRhs>&, std::index_sequence<Is...>)
{
  using T = decltype(typename TLhs::value_type() + typename TRhs::value_type());
  using TOut = std::integer_sequence<T, (get_or<Is, 0>(Vector<TLhs>()) + get_or<Is, 0>(Vector<TRhs>()))...>;
  return Vector<TOut>();
}

} // namespace Impl

template <typename TLhs, typename TRhs>
auto operator+(Vector<TLhs> lhs, const Vector<TRhs>& rhs)
{
  if constexpr (Impl::VectorTraits<TLhs>::is_zero) {
    return rhs;
  } else if constexpr (Impl::VectorTraits<TRhs>::is_zero) {
    return lhs;
  } else if constexpr (Impl::VectorTraits<TLhs>::has_static_coefs && Impl::VectorTraits<TRhs>::has_static_coefs) {
    return Impl::static_add_impl(lhs, rhs, std::make_index_sequence<std::max(TLhs::size(), TRhs::size())>());
  } else {
    for (std::size_t i = 0; i < lhs.size(); ++i) {
      lhs[i] += rhs[i];
    }
    return lhs;
  }
}

} // namespace Linx
