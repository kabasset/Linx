// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_VIEWS_H
#define LINX_BASE_VIEWS_H

#include "Linx/Base/Types.h"

#include <Kokkos_Core.hpp>
#include <Kokkos_DynRankView.hpp>
#include <Kokkos_OffsetView.hpp>

namespace Linx {

static constexpr int kokkos_max_rank = 8; ///< Maximum rank of a `Kokkos::View`
static constexpr int kokkos_max_dyn_rank = 7; ///< Maximum rank of a `Kokkos::DynRankView`
static constexpr int kokkos_max_op_rank = 6; ///< Maximum rank of most Kokkos operations

/**
 * @brief Default sequence view instance.
 */
template <typename T, int N, typename... TArgs>
auto default_sequence_view()
{
  if constexpr (N == -1 || N == 0) {
    return Kokkos::View<T*, TArgs...>();
  } else {
    return Kokkos::View<T[N], TArgs...>();
  }
}

/**
 * @brief Default sequence view type.
 */
template <typename T, int N, typename... TArgs>
using SequenceView = decltype(default_sequence_view<T, N, TArgs...>());

/**
 * @brief Default image view instance.
 */
template <typename T, typename TDomain, typename... TArgs>
static constexpr auto default_image_view()
{
  constexpr auto n = TDomain::n;
  static_assert(kokkos_max_rank <= 8);
  static_assert(n <= kokkos_max_rank);

#define N(I) (TDomain::stop_type::at(I) - TDomain::start_type::at(I))

  // We avoid recursion to make NVCC happier
  if constexpr (n == -1) {
    return Kokkos::DynRankView<T, TArgs...>();
  } else if constexpr (n == 0) {
    return Kokkos::View<T, TArgs...>(); // Scalar
  } else if constexpr (n == 1) {
    if constexpr (TDomain::static_flag) {
      if constexpr (N(0) == 0) { // TODO perform test for each axis?
        return Kokkos::View<T*, TArgs...>();
      } else {
        return Kokkos::View<T[N(0)], TArgs...>();
      }
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
 * @brief Default image view type.
 */
template <typename T, typename TDomain, typename... TArgs>
using ImageView = decltype(default_image_view<T, TDomain, TArgs...>());

/**
 * @brief Traits to rebind views.
 */
template <typename T>
struct Rebind {
  template <typename U>
  using as_type = std::conditional_t<std::is_same_v<U, void>, T, U>; ///< New value type
  using add_const = const T; ///< Constant value type
};

/**
 * @brief Pointer specialization.
 */
template <typename T>
struct Rebind<T*> {
  template <typename U>
  using as_type = typename Rebind<T>::as_type<U>*; ///< New pointed type
  using add_const = typename Rebind<T>::add_const*; ///< Read-only pointer
};

/**
 * @brief Array specialization.
 */
template <typename T, std::size_t N>
struct Rebind<T[N]> {
  template <typename U>
  using as_type = typename Rebind<T>::as_type<U>[N]; ///< Array of new type
  using add_const = typename Rebind<T>::add_const[N]; ///< Read-only array
};

/**
 * @brief `View` specialization.
 */
template <typename TData, typename... TArgs>
struct Rebind<Kokkos::View<TData, TArgs...>> {
  template <typename U>
  using as_type = Kokkos::View<typename Rebind<TData>::as_type<U>, TArgs...>; ///< View of new type
  using add_const = Kokkos::View<typename Rebind<TData>::add_const, TArgs...>; ///< Read-only view
  using add_random_access =
      Kokkos::View<TData, TArgs..., Kokkos::MemoryTraits<Kokkos::RandomAccess>>; ///< Random-access view
  using add_atomic = Kokkos::View<TData, TArgs..., Kokkos::MemoryTraits<Kokkos::Atomic>>; ///< Atomic-access view
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
  return Kokkos::View<typename Rebind<TData>::as_type<U>, TArgs...>(label, in.layout());
}

/**
 * @brief Get a read-only view.
 * 
 * This is a no-op if the view is already read-only.
 */
template <typename TData, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_const(const Kokkos::View<TData, TArgs...>& in)
{
  if constexpr (std::is_const_v<typename Kokkos::View<TData, TArgs...>::value_type>) {
    return in;
  } else {
    using Out = typename Rebind<Kokkos::View<TData, TArgs...>>::add_const;
    return Out(in);
  }
}

/**
 * @brief Get a read-only view optimized for random access.
 */
template <typename TData, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_texture(const Kokkos::View<TData, TArgs...>& in)
{
  using Out = typename Rebind<typename Rebind<Kokkos::View<TData, TArgs...>>::add_const>::add_random_access;
  return Out(in);
}

/**
 * @brief Get an atomic-access view.
 */
template <typename TData, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_atomic(const Kokkos::View<TData, TArgs...>& in)
{
  using Out = typename Rebind<Kokkos::View<TData, TArgs...>>::add_atomic;
  return Out(in);
}

/**
 * @brief `DynRankView` specialization.
 */
template <typename TData, typename... TArgs>
struct Rebind<Kokkos::DynRankView<TData, TArgs...>> {
  template <typename U>
  using as_type = Kokkos::DynRankView<typename Rebind<TData>::as_type<U>, TArgs...>; ///< View of new type
  using add_const = Kokkos::DynRankView<typename Rebind<TData>::add_const, TArgs...>; ///< Read-only view
  using add_random_access =
      Kokkos::DynRankView<TData, TArgs..., Kokkos::MemoryTraits<Kokkos::RandomAccess>>; ///< Random-access view
  using add_atomic = Kokkos::DynRankView<TData, TArgs..., Kokkos::MemoryTraits<Kokkos::Atomic>>; ///< Atomic-access view
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
  return Kokkos::DynRankView<typename Rebind<TData>::as_type<U>, TArgs...>(label, in.layout());
}

/**
 * @brief Get a read-only view.
 * 
 * This is a no-op if the view is already read-only.
 */
template <typename TData, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_const(const Kokkos::DynRankView<TData, TArgs...>& in)
{
  if constexpr (std::is_const_v<typename Kokkos::DynRankView<TData, TArgs...>::value_type>) {
    return in;
  } else {
    using Out = typename Rebind<Kokkos::DynRankView<TData, TArgs...>>::add_const;
    return Out(in);
  }
}

/**
 * @brief Get a read-only view optimized for random access.
 */
template <typename TData, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_texture(const Kokkos::DynRankView<TData, TArgs...>& in)
{
  using Out = typename Rebind<typename Rebind<Kokkos::DynRankView<TData, TArgs...>>::add_const>::add_random_access;
  return Out(in);
}

/**
 * @brief Get an atomic-access view.
 */
template <typename TData, typename... TArgs>
KOKKOS_INLINE_FUNCTION decltype(auto) as_atomic(const Kokkos::DynRankView<TData, TArgs...>& in)
{
  using Out = typename Rebind<Kokkos::DynRankView<TData, TArgs...>>::add_atomic;
  return Out(in);
}

/**
 * @brief Any type `T` with an `as_const(const T&)` overload.
 */
template <typename T>
concept ViewableAsReadonly = requires(const T& in) { as_const(in); };

/**
 * @brief Any type `T` for which `as_const(const T&)` should not be applied.
 * 
 * This encompasses types without an `as_const(const T&)` overload
 * and those with a const-qualified `T::element_type`.
 */
template <typename T>
concept DontApplyReadonly = not ViewableAsReadonly<T> || std::is_const_v<typename T::element_type>;

/**
 * @brief Any type `T` for which `as_const(const T&)` should be applied.
 * 
 * This is the negation of `DontApplyReadonly`.
 */
template <typename T>
concept ApplyReadonly = not DontApplyReadonly<T>;

/**
 * @brief Return `as_const(in)` if applicable, `in` otherwise.
 */
template <typename T>
decltype(auto) try_as_const(const T& in)
{
  if constexpr (ApplyReadonly<T>) {
    return as_const(in);
  } else {
    return LINX_FORWARD(in);
  }
}

/**
 * @brief Try applying `as_texture()` or `as_const()`.
 * 
 * `as_texture(in)` is returned if defined.
 * Otherwise, `as_const(in)` is returned if defined.
 * Otherwise, `in` is returned.
 */
template <typename T>
decltype(auto) try_as_texture(const T& in)
{
  if constexpr (requires { as_texture(in); }) {
    return as_texture(in);
  } else if constexpr (requires { as_const(in); }) {
    return as_const(in);
  } else {
    return in;
  }
}

} // namespace Linx

#endif
