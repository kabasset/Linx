// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_SEQUENCE_FUNCS_H
#define LINX_DATA_SEQUENCE_FUNCS_H

#include "Linx/Base/Slice.h"
#include "Linx/Data/Sequence/creation.h"

#include <Kokkos_Core.hpp>

namespace Linx {

/**
 * @brief Copy as many elements as possible from `in` to `out`.
 */
template <typename TIn, typename TOut>
void copy_to(const TIn& in, const TOut& out) // FIXME rm
{
  auto domain = Slice(0, std::min<int>(std::size(in), std::size(out)));
  for_each<typename TOut::execution_space>("copy_to()", domain, KOKKOS_LAMBDA(int i) { out(i) = in(i); });
}

/**
 * @ingroup creation
 * @brief Copy an array as a static-size sequence.
 * @tparam N The static size
 * @tparam TSpace The memory space
 * @param label The sequence label
 * @param in The input array
 * 
 * If the input array is larger than `N`, only the `N` first values are copied.
 * If it is smaller, the remaining values are default-initialized.
 */
template <int N, typename TSpace = Kokkos::DefaultExecutionSpace>
auto resize(const std::string& label, const Specialization<Image> auto& in)
{
  static_assert(N >= 0);
  using T = LINX_DECLTYPE(in(0));
  auto out = no_init<T, N, TSpace>(label);
  copy_to(in, out);
  return out;
}

/**
 * @ingroup creation
 * @brief Copy an array as a dynamic-size sequence.
 * @tparam TSpace The memory space
 * @param size The dynamic size
 * @param label The sequence label
 * @param in The input array
 * 
 * If the input array is larger than `size`, only the `size` first values are copied.
 * If it is smaller, the remaining values are default-initialized.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
auto resize(std::integral auto size, const std::string& label, const Specialization<Image> auto& in)
{
  using T = LINX_DECLTYPE(in(0));
  auto out = no_init<T, TSpace>(size, label);
  copy_to(in, out);
  return out;
}

} // namespace Linx

#endif
