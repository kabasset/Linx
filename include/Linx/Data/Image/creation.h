// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_IMAGE_CREATION_H
#define LINX_DATA_IMAGE_CREATION_H

#include "Linx/Data/Box.h"
#include "Linx/Data/Image/types.h"

#include <Kokkos_Core.hpp>

namespace Linx {

/**
 * @ingroup creation
 * @brief Create a default-initialized image.
 * @tparam T The value type
 * @tparam TSpace The memory space
 * @param label The label
 * @param domain The domain
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
auto default_init(const std::string& label, const Specialization<Box> auto& domain)
{
  using Domain = LINX_DECLTYPE(domain);
  return Image<T, Domain, ImageView<T, Domain, TSpace>>(label, domain);
}

/**
 * @ingroup creation
 * @brief Create a default-initialized image.
 * @tparam T The value type
 * @tparam TSpace The memory space
 * @param label The label
 * @param extents The domain extents
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
auto default_init(const std::string& label, std::integral auto... extents)
{
  using Domain = decltype(shape(extents...));
  return Image<T, Domain, ImageView<T, Domain, TSpace>>(label, extents...);
}

/**
 * @ingroup creation
 * @brief Create an uninitialized image.
 * @tparam T The value type
 * @tparam TSpace The memory space
 * @param label The label
 * @param domain The domain
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
auto no_init(const std::string& label, const Specialization<Box> auto& domain)
{
  using Domain = LINX_DECLTYPE(domain);
  return Image<T, Domain, ImageView<T, Domain, TSpace>>(label, domain);
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
  return Image<T, Domain, ImageView<T, Domain, TSpace>>(
      Forward(),
      Kokkos::view_alloc(label, Kokkos::WithoutInitializing),
      extents...);
}

/**
 * @ingroup creation
 * @brief Create a 1D image made of a single row.
 * @tparam TSpace The memory space
 * @param label The label
 * @param row The array of values
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T, int N>
auto rowwise(const std::string& label, T (&&row)[N])
{
  auto raster = Raster<T, 1>(Wrap(row), N);
  auto out = no_init<T, TSpace>(label, shape<N>());
  Kokkos::deep_copy(out.base(), raster.base());
  return out;
}

/**
 * @ingroup creation
 * @brief Create a 2D image from a collection of rows.
 * @tparam TSpace The memory space
 * @param label The label
 * @param rows The array of arrays of values
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T, int N0, int N1>
auto rowwise(const std::string& label, T (&&rows)[N1][N0])
{
  auto raster = Raster<T, 2>(Wrap(*rows), N0, N1);
  auto out = no_init<T, TSpace>(label, shape<N0, N1>());
  Kokkos::deep_copy(out.base(), raster.base());
  return out;
}

/**
 * @ingroup creation
 * @brief Create a 3D image from a collection of rows.
 * @tparam TSpace The memory space
 * @param label The label
 * @param rows The array of arrays of arrays of values
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T, int N0, int N1, int N2>
auto rowwise(const std::string& label, T (&&rows)[N2][N1][N0])
{
  auto raster = Raster<T, 3>(Wrap(**rows), N0, N1, N2);
  auto out = no_init<T, TSpace>(label, shape<N0, N1, N2>());
  Kokkos::deep_copy(out.base(), raster.base());
  return out;
}

/**
 * @ingroup creation
 * @brief Wrap a pointer as a raster.
 * @param data The pointer
 * @param extents The domain extents
 */
template <typename T>
auto wrap(T* data, std::integral auto... extents)
{
  return Raster<T, sizeof...(extents)>(Wrap(data), extents...);
}

/**
 * @ingroup creation
 * @brief Wrap a pointer as a raster.
 * @param data The pointer
 * @param domain The domain
 */
template <typename T>
auto wrap(T* data, const Specialization<Box> auto& domain)
{
  return Image<T, LINX_DECLTYPE(domain), RasterView<T, LINX_DECLTYPE(domain)::n>>(Wrap(data), domain);
}

/**
 * @ingroup creation
 * @brief Create an image filled with a single value.
 * @tparam TSpace The memory space
 * @param label The label
 * @param value The value
 * @param domain The domain or extents
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

template <typename T>
struct IsOrigin {
  KOKKOS_INLINE_FUNCTION constexpr T operator()(std::integral auto... is) const
  {
    return ((is == 0) && ...);
  }
};

} // namespace Impl

/**
 * @ingroup creation
 * @brief Generate an image.
 * @tparam TSpace The memory space
 * @param label The label
 * @param func The generator
 * @param extents The domain extents
 * 
 * @warning Dynamic ranks are not supported.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
auto generate(const std::string& label, const auto& func, std::integral auto... extents)
{
  using T = LINX_DECLTYPE(Impl::apply_at_stop(func, extents...));
  return no_init<T, TSpace>(label, LINX_FORWARD(extents)...).copy_from(func);
}

template <typename TSpace = Kokkos::DefaultExecutionSpace>
auto generate(const std::string& label, const auto& func, const Specialization<Box> auto& domain)
{
  using T = LINX_DECLTYPE(Impl::apply_at_stop(func, domain));
  return no_init<T, TSpace>(label, domain).copy_from(func);
}

/**
 * @ingroup creation
 * @brief Generate an impulse.
 * @tparam T The value type
 * @tparam TSpace The memory space
 * @param label The label
 * @param domain The domain or extents
 * 
 * The generated image is filled with `T(false)` except at origin where it is `T(true)`.
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
auto impulse(const std::string& label, auto&&... domain)
{
  return generate(label, Impl::IsOrigin<T>(), LINX_FORWARD(domain)...);
}

/**
 * @ingroup creation
 * @brief Create an image with the same memory layout as another image.
 * @tparam TRebind The type of the elements in the new image (defaults to the type of the elements in the input image)
 * @param label The label
 * @param in The input image
 */
template <typename TRebind = void, typename T, typename TDomain, typename TView>
auto same_layout(const std::string& label, const Image<T, TDomain, TView>& in)
{
  return Image<typename Rebind<T>::as_type<TRebind>, TDomain, typename Rebind<TView>::as_type<TRebind>>(
      in.domain(),
      Forward(),
      same_layout<TRebind>(label, in.base()));
}

} // namespace Linx

#endif
