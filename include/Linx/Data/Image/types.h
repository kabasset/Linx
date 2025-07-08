// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

namespace Linx {

/**
 * @brief Contiguous image on host with row-major ordering.
 * 
 * This specialization is mostly provided for interfacing with legacy code.
 * Row-major ordering means that the elements are contiguous along the first index,
 * which is conventionally considered to be the index along a row:
 * 
 * \code
 * Raster<int, 2> raster(shape);
 * assert(&raster(x, y) + 1 == &raster(x + 1, y));
 * \endcode
 * 
 * Said otherwise, the stride along axis 0 is 1.
 */
template <typename T, int N = 2>
using Raster = Image<T, N, ImageContainer<T, N, Kokkos::LayoutLeft, Kokkos::HostSpace>>;

template <typename T, std::integral... TExtents>
Image(Wrap<T*>, TExtents...) -> Image<T, sizeof...(TExtents)>;

template <typename T, typename U, int N, typename TContainer>
Image(Wrap<T*>, Sequence<U, N, TContainer>) -> Image<T, N>;

template <typename T, typename U, int N>
Image(Wrap<T*>, U (&&)[N]) -> Image<T, N>;

template <typename T>
struct IsImage : std::false_type {};

template <typename T, int N, typename... TArgs>
struct IsImage<Image<T, N, TArgs...>> : std::true_type {};

template <typename T>
concept AnyImage = IsImage<T>::value; // is_specialization won't work with non-type template parameters

} // namespace Linx
