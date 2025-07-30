// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_BOX_FUNCS_H
#define LINX_DATA_BOX_FUNCS_H

namespace Linx {

/**
 * @brief Stream insertion.
 */
template <typename TStart, typename TStop>
std::ostream& operator<<(std::ostream& os, const Box<TStart, TStop>& box)
{
  return os << box.start() << " ~ " << box.stop();
}

/**
 * @relatesalso Box
 * @brief Get the 1D span along the i-th axis.
 */
template <int I, typename TStart, typename TStop>
constexpr auto get(const Box<TStart, TStop>& box)
{
  return Slice(box.start(I), box.stop(I));
}

namespace Impl {

template <typename TSlice, std::size_t... Is>
constexpr auto box_impl(const TSlice& slice, std::index_sequence<Is...>)
{
  return Box(vec(get<Is>(slice).start()...), vec(get<Is>(slice).stop()...));
}

} // namespace Impl

/**
 * @relatesalso Box
 * @brief Get the bounding box of a box.
 * 
 * This function is a no-op, it merely forwards its input.
 */
template <typename TStart, typename TStop>
constexpr const Box<TStart, TStop>& bbox(const Box<TStart, TStop>& in)
{
  return in;
}

/**
 * @relatesalso Slice
 * @brief Get the bounding box of a slice.
 * 
 * @warning Unbounded slices are not supported, and singleton slices must be integral.
 */
template <typename T, typename... TFuncs>
constexpr auto bbox(const Slice<T, TFuncs...>& slice)
{
  return Impl::box_impl(slice, std::make_index_sequence<sizeof...(TFuncs)>());
}

/**
 * @brief Set the static rank of a box.
 * 
 * If `N` is larger than the box rank, bounds are padded with default-initialized values.
 * If `N` is smaller than the box rank, they are truncated.
 */
template <int N, typename TStart, typename TStop>
constexpr auto rerank(const Box<TStart, TStop>& in)
{
  return Box(resize<N>(in.start()), resize<N>(in.stop()));
}

namespace Impl {

template <typename TSpace, typename TStart, typename TStop, std::size_t... Is>
auto kokkos_execution_policy_impl(const Box<TStart, TStop>& domain, std::index_sequence<Is...>)
{
  using Policy = Kokkos::MDRangePolicy<TSpace, Kokkos::Rank<Box<TStart, TStop>::n>, Kokkos::IndexType<Index>>;
  using Array = Policy::point_type;
  return Policy(Array {domain.start(Is)...}, Array {domain.stop(Is)...});
}

} // namespace Impl

/**
 * @brief Get the execution policy of a box.
 */
template <typename TSpace, typename TStart, typename TStop>
auto kokkos_execution_policy(const Box<TStart, TStop>& domain)
{
  // TODO support Properties?
  if constexpr (Box<TStart, TStop>::n == 1) {
    return Kokkos::RangePolicy<TSpace, Kokkos::IndexType<Index>>(domain.start(0), domain.stop(0));
  } else {
    return Impl::kokkos_execution_policy_impl<TSpace>(domain, std::make_index_sequence<Box<TStart, TStop>::n>());
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
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename TStart, typename TStop, typename TFunc>
void for_each(const std::string& label, const Box<TStart, TStop>& region, TFunc&& func)
{
#define LINX_CASE_RANK(n) \
  case n: \
    if constexpr (is_nary<TFunc, int, n>()) { \
      return Kokkos::parallel_for(label, kokkos_execution_policy<TSpace>(rerank<n>(region)), LINX_FORWARD(func)); \
    } else { \
      return; \
    }

  if constexpr (Box<TStart, TStop>::n == -1) {
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
