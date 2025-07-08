// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

namespace Linx {

/**
 * @ingroup creation
 * @brief Create a 1D image made of a single row.
 */
template <typename T, int N>
auto rowwise(const std::string& label, T (&&row)[N])
{
  auto raster = Raster<T, 1>(Wrap(row), N);
  auto out = Image<T, 1>(label, N);
  Kokkos::deep_copy(out.container(), raster.container());
  return out;
}

/**
 * @ingroup creation
 * @brief Create a 2D image from a collection of rows.
 */
template <typename T, int N0, int N1>
auto rowwise(const std::string& label, T (&&rows)[N1][N0])
{
  T* data = *rows;
  auto raster = Raster<T, 2>(Wrap(data), N0, N1);
  auto out = Image<T, 2>(label, N0, N1);
  Kokkos::deep_copy(out.container(), raster.container());
  return out;
}

/**
 * @ingroup creation
 * @brief Create a 3D image from a collection of rows.
 */
template <typename T, int N0, int N1, int N2>
auto rowwise(const std::string& label, T (&&rows)[N2][N1][N0])
{
  T* data = **rows;
  auto raster = Raster<T, 3>(Wrap(data), N0, N1, N2);
  auto out = Image<T, 3>(label, N0, N1, N2);
  Kokkos::deep_copy(out.container(), raster.container());
  return out;
}

/**
 * @ingroup creation
 * @brief Image filled with a single value.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T, std::integral... Is>
auto fill(const std::string& label, const T& value, Is... shape)
{
  static constexpr auto n = sizeof...(Is);
  return Image<T, n, ImageContainer<T, n, TSpace>>(label, shape...).fill(value); // TODO uninitialized
}

/**
 * @ingroup creation
 * @brief Generate an image.
 * @param label The label
 * @param func The generator
 * @param shape The shape
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
auto generate(const std::string& label, const auto& func, std::integral auto... shape)
{
  using T = std::remove_cvref_t<decltype(func(shape...))>;
  static constexpr auto n = sizeof...(shape);
  return Image<T, n, ImageContainer<T, n, TSpace>>(label, shape...).copy_from(func);
}

} // namespace Linx
