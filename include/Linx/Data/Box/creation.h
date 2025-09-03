// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_BOX_CREATION_H
#define LINX_DATA_BOX_CREATION_H

namespace Linx {

template <typename TVec>
auto origin()
{
  if constexpr (TVec::static_size_flag) {
    return vec<Dimension {TVec::n}>();
  } else {
    return TVec();
  }
}

template <typename TData>
using OriginData = typename decltype(origin<Vector<TData>>())::data_type;

/**
 * @relates Box
 * @brief Null rank.
 */
template <typename T = int>
Box() -> Box<std::integer_sequence<T>, std::integer_sequence<T>>;

/**
 * @relates Box
 * @brief Start at origin.
 */
template <typename T, int N>
Box(T (&&)[N]) -> Box<OriginData<T[N]>, T[N]>;

/**
 * @relates Box
 * @brief Start at origin
 */
template <typename TStop>
Box(const Vector<TStop>&) -> Box<OriginData<TStop>, TStop>;

/**
 * @relates Box
 * @brief Specify start and stop bounds.
 */
template <typename T0, int N0, typename T1, int N1>
Box(T0 (&&)[N0], T1 (&&)[N1]) -> Box<T0[N0], T1[N1]>; // FIXME forbid different N's?

/**
 * @relatesalso Box
 * @brief Create a box which starts at origin.
 * 
 * This is a shortcut for `Box(vec<Args...>(args...))`.
 */
template <auto... Args>
constexpr auto shape(auto... args)
{
  if constexpr (sizeof...(Args) == 0 && sizeof...(args) == 0) {
    return Box<std::integer_sequence<int>, std::integer_sequence<int>>();
  } else {
    auto stop = vec<Args...>(args...);
    if constexpr (decltype(stop)::static_size_flag) {
      return Box(vec<Dimension {decltype(stop)::n}>(), stop);
    } else {
      return Box(vec(Dimension(stop.size())), stop);
    }
  }
}

/**
 * @relatesalso Box
 * @brief Create a box centered at origin.
 * @param radius The radius vector
 * 
 * The extent of the box along axis `i` is `2 * radius[i] + 1`.
 */
template <typename T>
constexpr auto cuboid(const Vector<T>& radius)
{
  // TODO handle floating point coefficients
  return Box(-radius, radius + StaticConstant<1>());
}

/**
 * @relatesalso Box
 * @brief Create a dynamic-rank cube centered at origin.
 * @param d The box rank
 * @param radius The cube radius
 * 
 * The box extent along each axis is `2 * radius + 1`.
 */
constexpr auto cube(Dimension d, auto radius)
{
  return cuboid(vec(d, radius));
}

/**
 * @relatesalso Box
 * @brief Create a static-rank cube centered at origin.
 * @tparam D The box rank
 * @param radius The cube radius
 * 
 * The box extent along each axis is `2 * radius + 1`.
 */
template <Dimension D>
constexpr auto cube(auto radius)
{
  return cuboid(vec<D>(radius));
}

/**
 * @relatesalso Box
 * @brief Create a static-size cube centered at origin.
 * @tparam D The box rank
 * @tparam Radius The cube radius
 * 
 * The box extent along each axis is `2 * Radius + 1`.
 */
template <Dimension D, std::integral auto Radius>
constexpr auto cube()
{
  return cuboid(vec<D, Radius>());
}

} // namespace Linx

#endif
