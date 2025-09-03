// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_VECTOR_FUNCS_H
#define LINX_DATA_VECTOR_FUNCS_H

namespace Linx {

/**
 * @brief Stream insertion.
 */
template <typename T>
std::ostream& operator<<(std::ostream& os, const Vector<T>& in)
{
  if (Vector<T>::static_zero_flag || in.size() == 0) {
    return os << "O";
  }

  os << "[" << in(0);
  for (std::size_t i = 1; i < in.size(); ++i) {
    os << ", " << in(i);
  }
  return os << "]";
}

/**
 * @brief Create a vector of given static size, padding with default-initialized values if necessary.
 */
template <int N, typename T>
constexpr auto resize(const Vector<T>& in)
{
  if constexpr (Vector<T>::static_flag) {
    Impl::static_add_impl(
        in,
        vec<Linx::Dimension {N}, typename Vector<T>::value_type()>(),
        std::make_index_sequence<N>());
  } else {
    auto out = Linx::vec<Linx::Dimension {N}>(typename Vector<T>::value_type());
    for (std::size_t i = 0; i < in.size(); ++i) {
      out[i] = in[i];
    }
    return out;
  }
}

} // namespace Linx

#endif
