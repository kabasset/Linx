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
  template <typename U>
  using As = std::conditional_t<std::is_same_v<U, void>, T, U>;
  using AsReadonly = const T;
};

/**
 * @brief Pointer specialization.
 */
template <typename T>
struct Rebind<T*> {
  template <typename U>
  using As = typename Rebind<T>::As<U>*;
  using AsReadonly = typename Rebind<T>::AsReadonly*;
};

/**
 * @brief `View` specialization.
 */
template <typename TData, typename... TArgs>
struct Rebind<Kokkos::View<TData, TArgs...>> {
  template <typename U>
  using As = Kokkos::View<typename Rebind<TData>::As<U>, TArgs...>;
  using AsReadonly = Kokkos::View<typename Rebind<TData>::AsReadonly, TArgs...>;
  using AsAtomic = Kokkos::View<TData, Kokkos::MemoryTraits<Kokkos::Atomic>, TArgs...>;
};

/**
 * @brief Create a view with same shape but different data type.
 */
template <typename U = void, typename TData, typename... TArgs>
decltype(auto) same_layout(const std::string& label, const Kokkos::View<TData, TArgs...>& in)
{
  return Kokkos::View<typename Rebind<TData>::As<U>, TArgs...>(label, in.layout());
}

/**
 * @brief Get a read-only view.
 */
template <typename TData, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_readonly(const Kokkos::View<TData, TArgs...>& in)
{
  if constexpr (std::is_const_v<TData>) { // FIXME rm pointer
    return in;
  } else {
    using Out = typename Rebind<Kokkos::View<TData, TArgs...>>::AsReadonly;
    return Out(in);
  }
}

/**
 * @brief Get an atomic view.
 */
template <typename TData, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_atomic(const Kokkos::View<TData, TArgs...>& in)
{
  using Out = typename Rebind<Kokkos::View<TData, TArgs...>>::AsAtomic;
  return Out(in);
}

/**
 * @brief `DynRankView` specialization.
 */
template <typename TData, typename... TArgs>
struct Rebind<Kokkos::DynRankView<TData, TArgs...>> {
  template <typename U>
  using As = Kokkos::DynRankView<typename Rebind<TData>::As<U>, TArgs...>;
  using AsReadonly = Kokkos::DynRankView<typename Rebind<TData>::AsReadonly, TArgs...>;
  using AsAtomic = Kokkos::DynRankView<TData, Kokkos::MemoryTraits<Kokkos::Atomic>, TArgs...>;
};

/**
 * @brief Create a view with same shape but different data type.
 */
template <typename U = void, typename TData, typename... TArgs>
decltype(auto) same_layout(const std::string& label, const Kokkos::DynRankView<TData, TArgs...>& in)
{
  return Kokkos::DynRankView<typename Rebind<TData>::As<U>, TArgs...>(label, in.layout());
}

template <typename TData, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_readonly(const Kokkos::DynRankView<TData, TArgs...>& in)
{
  if constexpr (std::is_const_v<TData>) { // FIXME rm pointer
    return in;
  } else {
    using Out = typename Rebind<Kokkos::DynRankView<TData, TArgs...>>::AsReadonly;
    return Out(in);
  }
}

template <typename TData, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_atomic(const Kokkos::DynRankView<TData, TArgs...>& in)
{
  using Out = typename Rebind<Kokkos::DynRankView<TData, TArgs...>>::AsAtomic;
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
