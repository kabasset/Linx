// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_CONCEPTS_ARRAY_H
#define LINX_BASE_CONCEPTS_ARRAY_H

namespace Linx {

/**
 * @brief An object with a unary integral subscript operator and a size.
 * @see `std::vector`, `Kokkos::Array`, `Sequence`...
 */
template <typename T>
concept LegacyArray = requires(const T a) {
  std::size(a);
  a[0];
};

/**
 * @brief A contiguous data container defined by its size and a pointer to its first element.
 * @see `Sequence`, `Raster`...
 */
template <typename T>
concept SizedData = requires(const T a) {
  std::size(a);
  a.data();
};

/**
 * @brief A function object which takes N integers as arguments.
 * @tparam N The rank, aka. arity
 * 
 * Special case `N = -1` cannot be processed at compile-time;
 * Only the presence of a unary call operator on `int` is tested, like for `N = 1`.
 * 
 * @see `Image`, `Map`, `Patch`, `GaussianRng`...
 */
template <typename T, int N>
concept IndexedFunc = is_nary<T, int, std::abs(N)>();

/**
 * @brief An indexed object with a bounded domain.
 */
template <typename T, int N>
concept BoundedFunc = IndexedFunc<T, N> && requires(const T f) {
  f.size();
  f.domain();
};

/**
 * @brief An indexed object with a stride for each dimension.
 */
template <typename T, int N>
concept StridedFunc = IndexedFunc<T, N> && requires(const T f) { f.stride(0); };

/**
 * @brief A `BoundedFunc` which is defined over a box-shaped domain.
 */
template <typename T, int N>
concept BoxedFunc = BoundedFunc<T, N> && requires(const T f) {
  f.extent(0);
  f.shape(); // FIXME useful?
};

// /**
//  * @brief A `BoxedFunc` which manages its memory.
//  * @see `Sequence` (1D), `Image` (ND).
//  */
// template <typename T, int N>
// concept Array = BoxedFunc<T, N> && requires(const T data) {
//   data.container(); // FIXME -> Kokkos View-like
// };

} // namespace Linx

#endif
