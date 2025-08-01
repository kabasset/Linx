// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_BOX_FUNCS_H
#define LINX_DATA_BOX_FUNCS_H

#include "Kokkos_Core.hpp"
#include "Linx/Base/Types.h" // Specialization

#include <Kokkos_Core.hpp>
#include <concepts>
#include <ostream>
#include <string>

namespace Linx {

/**
 * @relatesalso Box
 * @brief Stream insertion.
 */
std::ostream& operator<<(std::ostream& os, const Specialization<Box> auto& box)
{
  return os << box.start() << " ~ " << box.stop();
}

/**
 * @relatesalso Box
 * @brief 1D slice along the i-th axis.
 */
template <std::integral auto I>
constexpr auto get(const Specialization<Box> auto& box)
{
  return Slice(box.start(I), box.stop(I));
}

/**
 * @relatesalso Box
 * @brief Bounding box of a box.
 * 
 * This function is a no-op, it merely forwards its input.
 */
constexpr const auto& bbox(const Specialization<Box> auto& in)
{
  return in;
}

/**
 * @relatesalso Slice
 * @brief Bounding box of a slice.
 * 
 * @warning Unbounded slices are not supported, and singleton slices must be integral.
 */
constexpr auto bbox(const Specialization<Slice> auto& slice)
{
  return [=]<std::size_t... Is>(const auto& s, std::index_sequence<Is...>) {
    return Box(vec(get<Is>(s).start()...), vec(get<Is>(s).stop()...));
  }(slice, std::make_index_sequence<LINX_DECLTYPE(slice)::n>());
}

/**
 * @brief Set the static rank of a box.
 * 
 * If `N` is larger than the box rank, bounds are padded with default-initialized values.
 * If `N` is smaller than the box rank, they are truncated.
 */
template <std::integral auto N>
constexpr auto rerank(const Specialization<Box> auto& in)
{
  return Box(resize<N>(in.start()), resize<N>(in.stop()));
}

/**
 * @brief Execution policy of a box.
 */
template <typename TSpace> // TODO support Properties?
auto kokkos_execution_policy(const Specialization<Box> auto& domain)
{
  static constexpr auto n = LINX_DECLTYPE(domain)::n;
  if constexpr (n == 1) {
    return Kokkos::RangePolicy<TSpace, Kokkos::IndexType<Index>>(domain.start(0), domain.stop(0));
  } else {
    return [=]<std::size_t... Is>(const auto& d, std::index_sequence<Is...>) {
      using Policy = Kokkos::MDRangePolicy<TSpace, Kokkos::Rank<n>, Kokkos::IndexType<Index>>;
      using Array = Policy::point_type;
      return Policy(Array {d.start(Is)...}, Array {d.stop(Is)...});
    }(domain, std::make_index_sequence<n>());
  }
}

/**
 * @ingroup regions
 * @brief Apply a function to each position of a region.
 * 
 * @param label Some label for debugging
 * @param region The region
 * @param func The function
 * 
 * The coordinate type must be integral and the function must take integral coordinates as input.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
void for_each(const std::string& label, const Specialization<Box> auto& region, auto&& func)
{
#define LINX_CASE_RANK(n) \
  case n: \
    if constexpr (is_nary<LINX_DECLTYPE(func), int, n>()) { \
      return Kokkos::parallel_for(label, kokkos_execution_policy<TSpace>(rerank<n>(region)), LINX_FORWARD(func)); \
    } else { \
      return; \
    }

  if constexpr (LINX_DECLTYPE(region)::n == -1) {
    switch (region.rank()) {
      case 0:
        return;
        LINX_CASE_RANK(1)
        LINX_CASE_RANK(2)
        LINX_CASE_RANK(3)
        LINX_CASE_RANK(4)
        LINX_CASE_RANK(5)
        LINX_CASE_RANK(6)
      default:
        throw Linx::OutOfBounds("Dynamic rank", region.rank(), Segment<int>(0, 6));
    }
  } else {
    Kokkos::parallel_for(label, kokkos_execution_policy<TSpace>(region), LINX_FORWARD(func));
  }

#undef LINX_CASE_RANK
}

} // namespace Linx

#endif
