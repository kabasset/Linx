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

static constexpr int kokkos_max_rank = 8; ///< Maximum rank of a `Kokkos::View`
static constexpr int kokkos_max_dyn_rank = 7; ///< Maximum rank of a `Kokkos::DynRankView`
static constexpr int kokkos_max_op_rank = 6; ///< Maximum rank of most Kokkos operations

/**
 * @brief Default sequence container instance.
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
 * @brief Default sequence container type.
 */
template <typename T, int N, typename... TArgs>
using SequenceContainer = decltype(default_sequence_container<T, N, TArgs...>());

/**
 * @brief Default image container instance.
 */
template <typename T, typename TDomain, typename... TArgs>
auto default_image_container()
{
  static constexpr auto n = TDomain::n;
  static_assert(kokkos_max_rank <= 8);
  static_assert(n <= kokkos_max_rank);

#define N(I) (TDomain::Stop::at(I) - TDomain::Start::at(I))

  // We avoid recursion to make NVCC happier
  if constexpr (n == -1) {
    return Kokkos::DynRankView<T, TArgs...>();
  } else if constexpr (n == 0) {
    return Kokkos::View<T, TArgs...>(); // Scalar
  } else if constexpr (n == 1) {
    if constexpr (TDomain::static_flag) {
      return Kokkos::View<T[N(0)], TArgs...>();
    } else {
      return Kokkos::View<T*, TArgs...>();
    }
  } else if constexpr (n == 2) {
    if constexpr (TDomain::static_flag) {
      return Kokkos::View<T[N(0)][N(1)], TArgs...>();
    } else {
      return Kokkos::View<T**, TArgs...>();
    }
  } else if constexpr (n == 3) {
    if constexpr (TDomain::static_flag) {
      return Kokkos::View<T[N(0)][N(1)][N(2)], TArgs...>();
    } else {
      return Kokkos::View<T***, TArgs...>();
    }
  } else if constexpr (n == 4) {
    if constexpr (TDomain::static_flag) {
      return Kokkos::View<T[N(0)][N(1)][N(2)][N(3)], TArgs...>();
    } else {
      return Kokkos::View<T****, TArgs...>();
    }
  } else if constexpr (n == 5) {
    if constexpr (TDomain::static_flag) {
      return Kokkos::View<T[N(0)][N(1)][N(2)][N(3)][N(4)], TArgs...>();
    } else {
      return Kokkos::View<T*****, TArgs...>();
    }
  } else if constexpr (n == 6) {
    if constexpr (TDomain::static_flag) {
      return Kokkos::View<T[N(0)][N(1)][N(2)][N(3)][N(4)][N(5)], TArgs...>();
    } else {
      return Kokkos::View<T******, TArgs...>();
    }
  } else if constexpr (n == 7) {
    if constexpr (TDomain::static_flag) {
      return Kokkos::View<T[N(0)][N(1)][N(2)][N(3)][N(4)][N(5)][N(6)], TArgs...>();
    } else {
      return Kokkos::View<T*******, TArgs...>();
    }
  } else if constexpr (n == 8) {
    if constexpr (TDomain::static_flag) {
      return Kokkos::View<T[N(0)][N(1)][N(2)][N(3)][N(4)][N(5)][N(6)][N(7)], TArgs...>();
    } else {
      return Kokkos::View<T********, TArgs...>();
    }
  }

#undef N
}

/**
 * @brief Default image container type.
 */
template <typename T, typename TDomain, typename... TArgs>
using ImageContainer = decltype(default_image_container<T, TDomain, TArgs...>());

/**
 * @brief Traits to rebind containers.
 */
template <typename T>
struct Rebind {
  template <typename U>
  using As = std::conditional_t<std::is_same_v<U, void>, T, U>; ///< New value type
  using AsReadonly = const T; ///< Constant value type
};

/**
 * @brief Pointer specialization.
 */
template <typename T>
struct Rebind<T*> {
  template <typename U>
  using As = typename Rebind<T>::As<U>*; ///< New pointed type
  using AsReadonly = typename Rebind<T>::AsReadonly*; ///< Read-only pointer
};

/**
 * @brief Array specialization.
 */
template <typename T, std::size_t N>
struct Rebind<T[N]> {
  template <typename U>
  using As = typename Rebind<T>::As<U>[N]; ///< Array of new type
  using AsReadonly = typename Rebind<T>::AsReadonly[N]; ///< Read-only array
};

/**
 * @brief `View` specialization.
 */
template <typename TData, typename... TArgs>
struct Rebind<Kokkos::View<TData, TArgs...>> {
  template <typename U>
  using As = Kokkos::View<typename Rebind<TData>::As<U>, TArgs...>; ///< View of new type
  using AsReadonly = Kokkos::View<typename Rebind<TData>::AsReadonly, TArgs...>; ///< Read-only view
  using AsAtomic = Kokkos::View<TData, TArgs..., Kokkos::MemoryTraits<Kokkos::Atomic>>; ///< Atomic-access view
};

/**
 * @brief Create a view with same layout and possibly different data type.
 * @tparam U The new value type or `void`
 * 
 * The new view has the same memory layout and shape.
 * The value type is also the same if `U` is void.
 */
template <typename U = void, typename TData, typename... TArgs>
decltype(auto) same_layout(const std::string& label, const Kokkos::View<TData, TArgs...>& in)
{
  return Kokkos::View<typename Rebind<TData>::As<U>, TArgs...>(label, in.layout());
}

/**
 * @brief Get a read-only view.
 * 
 * This is a no-op if the view is already read-only.
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
 * @brief Get an atomic-access view.
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
  using As = Kokkos::DynRankView<typename Rebind<TData>::As<U>, TArgs...>; ///< View of new type
  using AsReadonly = Kokkos::DynRankView<typename Rebind<TData>::AsReadonly, TArgs...>; ///< Read-only view
  using AsAtomic = Kokkos::DynRankView<TData, TArgs..., Kokkos::MemoryTraits<Kokkos::Atomic>>; ///< Atomic-access view
};

/**
 * @brief Create a view with same layout and possibly different data type.
 * @tparam U The new value type or `void`
 * 
 * The new view has the same memory layout and shape.
 * The value type is also the same if `U` is void.
 */
template <typename U = void, typename TData, typename... TArgs>
decltype(auto) same_layout(const std::string& label, const Kokkos::DynRankView<TData, TArgs...>& in)
{
  return Kokkos::DynRankView<typename Rebind<TData>::As<U>, TArgs...>(label, in.layout());
}

/**
 * @brief Get a read-only view.
 * 
 * This is a no-op if the view is already read-only.
 */
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

/**
 * @brief Get an atomic-access view.
 */
template <typename TData, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_atomic(const Kokkos::DynRankView<TData, TArgs...>& in)
{
  using Out = typename Rebind<Kokkos::DynRankView<TData, TArgs...>>::AsAtomic;
  return Out(in);
}

/**
 * @brief Any type `T` with an `as_readonly(const T&)` overload.
 */
template <typename T>
concept ViewableAsReadonly = requires(const T& in)
{
  as_readonly(in);
};

/**
 * @brief Any type `T` for which `as_readonly(const T&)` should not be applied.
 * 
 * This encompasses types without an `as_readonly(const T&)` overload
 * and those with a const-qualified `T::value_type`.
 */
template <typename T>
concept DontApplyReadonly = not ViewableAsReadonly<T> || std::is_const_v<typename T::value_type>;

/**
 * @brief Any type `T` for which `as_readonly(const T&)` should be applied.
 * 
 * This is the negation of `DontApplyReadonly`.
 */
template <typename T>
concept ApplyReadonly = not DontApplyReadonly<T>;

/**
 * @brief Return `as_readonly(in)` if applicable, `in` otherwise.
 */
template <typename T>
decltype(auto) try_as_readonly(const T& in)
{
  if constexpr (ApplyReadonly<T>) {
    return as_readonly(in);
  } else {
    return LINX_FORWARD(in);
  }
}

} // namespace Linx

#endif
