// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_IMAGE_CREATION_H
#define LINX_DATA_IMAGE_CREATION_H

#include "Linx/Data/Image/types.h"

namespace Linx {

/**
 * @ingroup creation
 * @brief Create a default-initialized image.
 * @tparam T The value type
 * @tparam TSpace The memory space
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
auto default_init(const std::string& label, const NotConvertibleTo<int> auto& domain)
{
  using Domain = LINX_DECLTYPE(domain);
  return Image<T, Domain, ImageContainer<T, Domain, TSpace>>(label, domain);
}

/**
 * @ingroup creation
 * @brief Create a default-initialized image.
 * @tparam T The value type
 * @tparam TSpace The memory space
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
auto default_init(const std::string& label, std::integral auto... extents)
{
  using Domain = decltype(shape(extents...));
  return Image<T, Domain, ImageContainer<T, Domain, TSpace>>(label, extents...);
}

/**
 * @ingroup creation
 * @brief Create an uninitialized image.
 * @tparam T The value type
 * @tparam TSpace The memory space
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
auto no_init(const std::string& label, const NotConvertibleTo<int> auto& domain)
{
  using Domain = LINX_DECLTYPE(domain);
  return Image<T, Domain, ImageContainer<T, Domain, TSpace>>(label, domain);
  // FIXME view_alloc(label, WithoutInitializing)
}

/**
 * @ingroup creation
 * @brief Create an uninitialized image.
 * @tparam T The value type
 * @tparam TSpace The memory space
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
auto no_init(const std::string& label, std::integral auto... extents)
{
  using Domain = decltype(shape(extents...));
  return Image<T, Domain, ImageContainer<T, Domain, TSpace>>(
      Forward(),
      Kokkos::view_alloc(label, Kokkos::WithoutInitializing),
      extents...);
}

/**
 * @ingroup creation
 * @brief Create a 1D image made of a single row.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T, int N>
auto rowwise(const std::string& label, T (&&row)[N])
{
  auto domain = shape<N>();
  auto raster = Raster<T, decltype(domain)>(Wrap(row), N);
  auto out = no_init<T, TSpace>(label, domain);
  Kokkos::deep_copy(out.container(), raster.container());
  return out;
}

/**
 * @ingroup creation
 * @brief Create a 2D image from a collection of rows.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T, int N0, int N1>
auto rowwise(const std::string& label, T (&&rows)[N1][N0])
{
  T* data = *rows;
  auto domain = shape<N0, N1>();
  auto raster = Raster<T, decltype(domain)>(Wrap(data), N0, N1);
  auto out = no_init<T, TSpace>(label, domain);
  Kokkos::deep_copy(out.container(), raster.container());
  return out;
}

/**
 * @ingroup creation
 * @brief Create a 3D image from a collection of rows.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T, int N0, int N1, int N2>
auto rowwise(const std::string& label, T (&&rows)[N2][N1][N0])
{
  T* data = **rows;
  auto domain = shape<N0, N1, N2>();
  auto raster = Raster<T, decltype(domain)>(Wrap(data), N0, N1, N2);
  auto out = no_init<T, TSpace>(label, domain);
  Kokkos::deep_copy(out.container(), raster.container());
  return out;
}

/**
 * @ingroup creation
 * @brief Wrap a pointer as a raster with given shape.
 */
template <typename T>
auto wrap(T* data, std::integral auto... extents)
{
  auto domain = shape(extents...);
  return Raster<T, decltype(domain)>(Wrap(data), extents...);
}

/**
 * @ingroup creation
 * @brief Wrap a pointer as a raster with given domain.
 */
template <typename T>
auto wrap(T* data, const NotConvertibleTo<int> auto& domain)
{
  return Raster<T, LINX_DECLTYPE(domain)>(Wrap(data), domain);
}

/**
 * @ingroup creation
 * @brief Image filled with a single value.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T>
auto fill(const std::string& label, const T& value, auto&&... domain)
{
  return no_init<T, TSpace>(label, LINX_FORWARD(domain)...).fill(value);
}

namespace Impl {

auto apply_at_stop(auto func, std::integral auto... stop)
{
  return func(stop...);
}

template <std::size_t... Is>
auto apply_at_stop_impl(auto func, const auto& domain, std::index_sequence<Is...>)
{
  return apply_at_stop(func, domain.stop(Is)...);
}

template <typename TDomain>
  requires(TDomain::n >= 0)
auto apply_at_stop(auto func, const TDomain& domain)
{
  return apply_at_stop_impl(func, domain, std::make_index_sequence<static_cast<std::size_t>(TDomain::n)>());
}

} // namespace Impl

/**
 * @ingroup creation
 * @brief Generate an image.
 * @param label The label
 * @param func The generator
 * @param domain The domain parameters
 * 
 * @warning Dynamic ranks are not supported.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
auto generate(const std::string& label, const auto& func, auto&&... domain)
{
  using T = LINX_DECLTYPE(Impl::apply_at_stop(func, domain...));
  return no_init<T, TSpace>(label, LINX_FORWARD(domain)...).copy_from(func);
}

/**
 * @ingroup creation
 * @brief Create an image with the same memory layout as another image.
 * @tparam TRebind The type of the elements in the new image (defaults to the type of the elements in the input image)
 */
template <typename TRebind = void, typename T, typename TDomain, typename TContainer>
auto same_layout(const std::string& label, const Image<T, TDomain, TContainer>& in)
{
  return Image<typename Rebind<T>::As<TRebind>, TDomain, typename Rebind<TContainer>::As<TRebind>>(
      in.domain(),
      Forward(),
      same_layout<TRebind>(label, in.container()));
}

} // namespace Linx

#endif
