// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_BOX_CREATION_H
#define LINX_DATA_BOX_CREATION_H

namespace Linx {

/**
 * @brief Start at origin.
 */
template <typename T, int N>
Box(T (&&)[N]) -> Box<std::integer_sequence<int>, T[N]>;

/**
 * @brief Specify start and stop bounds.
 */
template <typename T0, int N0, typename T1, int N1>
Box(T0 (&&)[N0], T1 (&&)[N1]) -> Box<T0[N0], T1[N1]>;

/**
 * @brief Create a box which starts at origin.
 * 
 * This is a shortcut for `Box(vec<Args...>(args...))`.
 */
template <auto... Args>
constexpr auto shape(auto... args)
{
  return Box(vec<Args...>(args...));
}

/**
 * @brief Create a box centered at origin.
 * @param radius The radius vector
 * 
 * If the radius coefficients are different, then the box is not regular.
 * The extent of the box along axis `i` is `2 * radius[i] + 1`.
 */
template <typename T>
constexpr auto cube(const Vector<T>& radius)
{
  // TODO handle floating point coefficients
  return Box(-radius, radius + StaticConstant<1>());
}

/**
 * @brief Create a static-rank, regular box centered at origin.
 * @tparam D The box rank
 * @param radius The cube radius
 * 
 * The box extent along each axis is `2 * radius + 1`.
 */
template <Dimension D>
constexpr auto cube(auto radius)
{
  return cube(vec<D>(radius));
}

/**
 * @brief Create a static-size, regular box centered at origin.
 * @tparam D The box rank
 * @tparam Radius The box radius
 * 
 * The box extent along each axis is `2 * Radius + 1`.
 */
template <Dimension D, std::integral auto Radius>
constexpr auto cube()
{
  return cube(vec<D, Radius>());
}

} // namespace Linx

#endif
