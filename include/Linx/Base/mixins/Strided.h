// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_MIXINS_STRIDED_H
#define LINX_BASE_MIXINS_STRIDED_H

#include "Linx/Base/Packs.h"

namespace Linx {

template <typename T>
concept Strided = requires(const T& in) {
  { in.stride(0) } -> std::integral;
};

namespace Impl {
/**
 * @brief Helper method to unroll indices.
 */
template <std::size_t... Is>
KOKKOS_INLINE_FUNCTION auto offset_impl(const auto& in, const auto& indices, std::index_sequence<Is...>)
{
  return ((get<Is>(indices) * in.stride(Is)) + ...);
}

} // namespace Impl

/**
 * @brief Address offset from the origin element of some strided input data container to the element at given indices.
 */
KOKKOS_INLINE_FUNCTION auto offset_from_origin(const Strided auto& in, std::integral auto... indices)
{
  return Impl::offset_impl(in, forward_as_tuple(indices...), std::make_index_sequence<sizeof...(indices)>());
}

} // namespace Linx

#endif