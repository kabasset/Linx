// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_CONTAINERS_H
#define LINX_BASE_CONTAINERS_H

#include "Linx/Base/Types.h"

#include <Kokkos_Core.hpp>
#include <Kokkos_DynRankView.hpp>
#include <Kokkos_OffsetView.hpp>

namespace Linx {

/**
 * Default sequence container instance.
 */
template <typename T, int N, typename... TArgs>
auto default_sequence_container()
{
  if constexpr (N == -1 || N == 0) {
    return Kokkos::View<T*, TArgs...>();
  } else {
    return Kokkos::View<T[N], TArgs...>();
  }
}

/**
 * Default sequence container type.
 */
template <typename T, int N, typename... TArgs>
using SequenceContainer = decltype(default_sequence_container<T, N, TArgs...>());

/**
 * Default image container instance.
 */
template <typename T, int N, typename... TArgs>
auto default_image_container()
{
  // We avoid recursion to make NVCC happier
  if constexpr (N == -1) {
    return Kokkos::DynRankView<T, TArgs...>();
  } else if constexpr (N == 0 || N == 1) {
    return Kokkos::View<T*, TArgs...>();
  } else if constexpr (N == 2) {
    return Kokkos::View<T**, TArgs...>();
  } else if constexpr (N == 3) {
    return Kokkos::View<T***, TArgs...>();
  } else if constexpr (N == 4) {
    return Kokkos::View<T****, TArgs...>();
  } else if constexpr (N == 5) {
    return Kokkos::View<T*****, TArgs...>();
  } else if constexpr (N == 6) {
    return Kokkos::View<T******, TArgs...>();
  } else if constexpr (N == 7) {
    return Kokkos::View<T*******, TArgs...>();
  } else if constexpr (N == 8) {
    return Kokkos::View<T********, TArgs...>();
  }
}

/**
 * Default image container type.
 */
template <typename T, int N, typename... TArgs>
using ImageContainer = decltype(default_image_container<T, N, TArgs...>());

/**
 * @brief Traits to rebind containers.
 */
template <typename T>
struct Rebind {
  using AsReadonly = const T;
};

/**
 * @brief Pointer specialization.
 */
template <typename T>
struct Rebind<T*> {
  using AsReadonly = typename Rebind<T>::AsReadonly*;
};

/**
 * @brief `View` specialization.
 */
template <typename T, typename... TArgs>
struct Rebind<Kokkos::View<T, TArgs...>> {
  template <typename U>
  using As = Kokkos::View<U, TArgs...>;
  using AsReadonly = Kokkos::View<typename Rebind<T>::AsReadonly, TArgs...>;
  using AsAtomic = Kokkos::View<T, Kokkos::MemoryTraits<Kokkos::Atomic>, TArgs...>;
};

/**
 * @brief Create a view with same shape but different data type.
 */
template <typename U, typename T, typename... TArgs>
decltype(auto) same_shape(const std::string& label, const Kokkos::View<T, TArgs...>& in)
{
  return Kokkos::View<U, TArgs...>(label, in.layout());
}

/**
 * @brief Get a read-only view.
 */
template <typename T, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_readonly(const Kokkos::View<T, TArgs...>& in)
{
  if constexpr (std::is_const_v<T>) {
    return in;
  } else {
    using Out = typename Rebind<Kokkos::View<T, TArgs...>>::AsReadonly;
    return Out(in);
  }
}

/**
 * @brief Get an atomic view.
 */
template <typename T, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_atomic(const Kokkos::View<T, TArgs...>& in)
{
  using Out = typename Rebind<Kokkos::View<T, TArgs...>>::AsAtomic;
  return Out(in);
}

/**
 * @brief `DynRankView` specialization.
 */
template <typename T, typename... TArgs>
struct Rebind<Kokkos::DynRankView<T, TArgs...>> {
  using AsReadonly = Kokkos::DynRankView<typename Rebind<T>::AsReadonly, TArgs...>;
  using AsAtomic = Kokkos::DynRankView<T, Kokkos::MemoryTraits<Kokkos::Atomic>, TArgs...>;
};

/**
 * @brief Create a view with same shape but different data type.
 */
template <typename U, typename T, typename... TArgs>
decltype(auto) same_shape(const std::string& label, const Kokkos::DynRankView<T, TArgs...>& in)
{
  return Kokkos::DynRankView<U, TArgs...>(label, in.layout());
}

template <typename T, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_readonly(const Kokkos::DynRankView<T, TArgs...>& in)
{
  if constexpr (std::is_const_v<T>) {
    return in;
  } else {
    using Out = typename Rebind<Kokkos::DynRankView<T, TArgs...>>::AsReadonly;
    return Out(in);
  }
}

template <typename T, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_atomic(const Kokkos::DynRankView<T, TArgs...>& in)
{
  using Out = typename Rebind<Kokkos::DynRankView<T, TArgs...>>::AsAtomic;
  return Out(in);
}

/**
 * @brief Specialization for standard containers with constant values.
 */
template <typename T>
const T& as_readonly(const T& in)
  requires(std::is_const_v<typename T::value_type>)
{
  return in;
}

} // namespace Linx

#endif
