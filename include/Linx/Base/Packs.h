// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_PACKS_H
#define LINX_BASE_PACKS_H

#include "Linx/Base/Types.h"

#include <tuple>
#include <type_traits>

namespace Linx {

/**
 * @brief Pack traits.
 */
template <typename T0, typename... Ts>
struct PackTraits {
  using First = T0;
  using Last = typename decltype((std::type_identity<Ts> {}, ...))::type;
};

template <typename T>
struct PackTraits<T> {
  using First = T;
  using Last = T;
};

namespace Impl {

template <typename TFunc, typename TTuple, std::size_t... Is>
KOKKOS_INLINE_FUNCTION auto apply_tuple_last_first(TFunc&& func, TTuple&& tuple, std::index_sequence<Is...>)
{
  constexpr auto N = std::tuple_size_v<TTuple>;
  return LINX_FORWARD(func)(std::get<N - 1>(LINX_FORWARD(tuple)), std::get<Is>(LINX_FORWARD(tuple))...);
}

template <typename TProj, typename TRed, typename TTuple, std::size_t... Is>
KOKKOS_INLINE_FUNCTION void
tuple_project_reduce_to(TProj&& projection, TRed&& reducer, TTuple&& tuple, std::index_sequence<Is...>)
{
  constexpr auto N = std::tuple_size_v<TTuple>;
  LINX_FORWARD(reducer).join(
      std::get<N - 1>(LINX_FORWARD(tuple)),
      LINX_FORWARD(projection)(std::get<Is>(LINX_FORWARD(tuple))...));
}

} // namespace Impl

template <typename TFunc, typename... Ts>
KOKKOS_INLINE_FUNCTION auto apply_last_first(TFunc&& func, Ts&&... args)
{
  return Impl::apply_tuple_last_first(
      LINX_FORWARD(func),
      std::forward_as_tuple(LINX_FORWARD(args)...),
      std::make_index_sequence<sizeof...(Ts) - 1> {});
}

template <typename TProj, typename TRed, typename... Ts>
KOKKOS_INLINE_FUNCTION void project_reduce_to(TProj&& projection, TRed&& reducer, Ts&&... args)
{
  Impl::tuple_project_reduce_to(
      LINX_FORWARD(projection),
      LINX_FORWARD(reducer),
      std::forward_as_tuple(LINX_FORWARD(args)...),
      std::make_index_sequence<sizeof...(Ts) - 1> {});
}

template <typename... Ts>
class Tuple {
public:

  static constexpr std::size_t size()
  {
    return 0;
  }
};

template <typename T0, typename... Ts>
class Tuple<T0, Ts...> {
public:

  KOKKOS_INLINE_FUNCTION Tuple(auto&& arg0, auto&&... args) :
      m_head {LINX_FORWARD(arg0)},
      m_tail {LINX_FORWARD(args)...}
  {}

  static constexpr std::size_t size()
  {
    return sizeof...(Ts) + 1;
  }

  KOKKOS_INLINE_FUNCTION constexpr const T0& front() const
  {
    return m_head;
  }

  KOKKOS_INLINE_FUNCTION constexpr const auto& back() const
  {
    return m_tail.back();
  }

  KOKKOS_INLINE_FUNCTION constexpr const Tuple<Ts...>& tail() const
  {
    return m_tail;
  }

private:

  T0 m_head;
  Tuple<Ts...> m_tail;
};

template <typename T>
class Tuple<T> {
public:

  KOKKOS_INLINE_FUNCTION Tuple(auto&& arg) : m_head {LINX_FORWARD(arg)} {}

  static constexpr std::size_t size()
  {
    return 1;
  }

  KOKKOS_INLINE_FUNCTION constexpr const T& front() const
  {
    return m_head;
  }

  KOKKOS_INLINE_FUNCTION constexpr const T& back() const
  {
    return m_head;
  }

  KOKKOS_INLINE_FUNCTION constexpr Tuple<> tail() const
  {
    return Tuple<> {};
  }

private:

  T m_head;
};

template <std::size_t I, typename T0, typename... Ts>
KOKKOS_INLINE_FUNCTION constexpr decltype(auto) get(const Tuple<T0, Ts...>& tuple)
{
  if constexpr (I == 0) {
    return tuple.front();
  } else {
    return get<I - 1>(tuple.tail());
  }
}

template <typename... Ts>
KOKKOS_INLINE_FUNCTION constexpr Tuple<Ts&&...> forward_as_tuple(Ts&&... args)
{
  return Tuple<Ts&&...>(LINX_FORWARD(args)...);
}

} // namespace Linx

#endif
