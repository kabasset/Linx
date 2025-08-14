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
 * 
 * If the inputs have different sizes, the number of elements copied is the minimum of the sizes.
 */
template <typename TIn, typename TOut>
void copy_max(const TIn& in, const TOut& out) // FIXME rm
{
  const auto in_on_device = on_device<typename TOut::memory_space>(in);
  auto domain = Slice(std::size_t(0), std::min<std::size_t>(std::size(in), std::size(out)));
  for_each<typename TOut::execution_space>(
      "copy_max()",
      domain,
      KOKKOS_LAMBDA(std::size_t i) { out(i) = in_on_device(i); });
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
  copy_max(in, out);
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
  copy_max(in, out);
  return out;
}

} // namespace Linx

#endif
