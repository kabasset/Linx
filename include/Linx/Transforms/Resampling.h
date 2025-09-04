// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_RESAMPLING_H
#define LINX_TRANSFORMS_RESAMPLING_H

#include "Linx/Base/Packs.h"
#include "Linx/Data/Image.h"

#include <concepts>
#include <string>

namespace Linx {

/**
 * @ingroup resampling
 * @brief Interpolated view.
 */
template <typename TParent, typename TMethod>
class Interpolation {
public:

  using parent_type = TParent;
  using Method = TMethod;

  Interpolation(parent_type parent, Method method) : m_parent(LINX_MOVE(parent)), m_method(LINX_MOVE(method)) {}

  std::string label() const
  {
    return compose_label("interpolate", m_parent, m_method);
  }

  KOKKOS_INLINE_FUNCTION const parent_type& parent() const
  {
    return m_parent;
  }

  KOKKOS_INLINE_FUNCTION const Method& method() const
  {
    return m_method;
  }

  KOKKOS_INLINE_FUNCTION auto operator()(std::floating_point auto... xs) const
  {
    return m_method(m_parent, xs...);
  }

  KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... is) const
  {
    return m_parent(is...);
  }

private:

  parent_type m_parent;
  Method m_method;
};

template <typename TSpace, typename TParent, typename TMethod>
decltype(auto) on_device(const Interpolation<TParent, TMethod>& in) // TODO to some ProxyMixin
{
  return Interpolation(on_device<TSpace>(in.parent()), in.method());
}

template <typename TParent, typename TMethod>
decltype(auto) on_host(const Interpolation<TParent, TMethod>& in) // TODO to some ProxyMixin
{
  return on_device<Kokkos::HostSpace>(in);
}

/**
 * @ingroup resampling
 * @brief Extrapolated view.
 */
template <typename TParent, typename TMethod>
class Extrapolation {
public:

  using parent_type = TParent;
  using Method = TMethod;
  using memory_space = TParent::memory_space;
  using execution_space = TParent::execution_space;

  Extrapolation(parent_type parent, Method method) : m_parent(LINX_MOVE(parent)), m_method(LINX_MOVE(method)) {}

  Extrapolation(parent_type parent, auto&&... args) : m_parent(LINX_MOVE(parent)), m_method {LINX_FORWARD(args)...} {}

  std::string label() const
  {
    return compose_label("extrapolate", m_parent, m_method);
  }

  KOKKOS_INLINE_FUNCTION const parent_type& parent() const
  {
    return m_parent;
  }

  KOKKOS_INLINE_FUNCTION const Method& method() const
  {
    return m_method;
  }

  KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... xs) const
  {
    return m_method(m_parent, xs...);
  }

private:

  parent_type m_parent;
  Method m_method;
};

template <typename TParent, typename TMethod>
auto as_const(const Extrapolation<TParent, TMethod>& in)
{
  return Extrapolation(try_as_const(in.parent()), in.method());
}

template <typename TSpace, typename TParent, typename TMethod>
decltype(auto) on_device(const Extrapolation<TParent, TMethod>& in) // TODO to some ProxyMixin
{
  return Extrapolation(on_device<TSpace>(in.parent()), in.method());
}

template <typename TParent, typename TMethod>
decltype(auto) on_host(const Extrapolation<TParent, TMethod>& in) // TODO to some ProxyMixin
{
  return on_device<Kokkos::HostSpace>(in);
}

template <typename T>
class Pad {
public:

  using value_type = T;

  Pad(value_type value) : m_value(value) {}

  std::string label() const
  {
    return "Pad";
  }

  KOKKOS_INLINE_FUNCTION const auto& operator()(const auto& in, std::integral auto... is) const
  {
    return extrapolate(in, forward_as_tuple(is...), std::make_index_sequence<sizeof...(is)>());
  }

private:

  template <typename TIn, typename TTuple, std::size_t... Is>
  KOKKOS_INLINE_FUNCTION const auto& extrapolate(const TIn& in, TTuple is, std::index_sequence<Is...>) const
  {
    return ((get<Is>(is) >= 0 && get<Is>(is) < in.extent(Is)) && ...) ? in(get<Is>(is)...) : m_value;
    // FIXME rely on TIn::domain_contains(is...)
  }

private:

  value_type m_value;
};

template <typename TParent, std::convertible_to<typename TParent::value_type> T>
Extrapolation(TParent, const T&) -> Extrapolation<TParent, Pad<T>>;

/**
 * @ingroup resampling
 * @brief Nearest-neighbor interpolation or extrapolation, a.k.a. zero-flux Neumann boundary conditions.
 */
struct Nearest {
  /**
   * @brief Return the value at the nearest in-bounds position.
   */
  KOKKOS_INLINE_FUNCTION const auto& operator()(const auto& in, std::integral auto... is) const
  {
    return extrapolate(in, forward_as_tuple(is...), std::make_index_sequence<sizeof...(is)>());
  }

  /**
   * @brief Return the value at the nearest integral position.
   */
  KOKKOS_INLINE_FUNCTION auto operator()(const auto& in, std::floating_point auto... xs) const
  {
    return in(Index(xs + 0.5)...);
  }

private:

  template <typename TIn, typename TTuple, std::size_t... Is>
  KOKKOS_INLINE_FUNCTION const auto& extrapolate(const TIn& in, TTuple is, std::index_sequence<Is...>)
  {
    return in(clamp(get<Is>(is), 0, in.extent(Is) - 1)...);
    // FIXME root(in)(clamp(get<Is>(is) - in.start(Is), 0, root(in).extent(Is)-1)...);
  }
};

/**
 * @ingroup resampling
 * @brief Periodic, a.k.a. symmetric or wrap-around, boundary conditions.
 */
struct Periodic {
  /**
   * @brief Return the value at the modulo position.
   */
  KOKKOS_INLINE_FUNCTION const auto& operator()(const auto& in, std::integral auto... is) const
  {
    return extrapolate(in, forward_as_tuple(is...), std::make_index_sequence<sizeof...(is)>());
  }

private:

  template <typename TIn, typename TTuple, std::size_t... Is>
  KOKKOS_INLINE_FUNCTION const auto& extrapolate(const TIn& in, TTuple is, std::index_sequence<Is...>)
  {
    auto unsigned_modulo = [](auto lhs, auto rhs) {
      auto out = lhs % rhs;
      return out < 0 ? out + rhs : out;
    }; // TODO functor?
    return in(unsigned_modulo(get<Is>(is), in.extent(Is))...);
    // FIXME support patches and offsets
  }
};

/**
 * @ingroup resampling
 * @brief Linear interpolation.
 */
struct Linear {
  KOKKOS_INLINE_FUNCTION auto operator()(const auto& in, std::floating_point auto... xs) const
  {
    return interpolate(in, forward_as_tuple(xs...));
  }

private:

  /**
   * @brief Return the interpolated value at given index.
   */
  template <typename TIn, typename TTuple, typename... TIndices>
  KOKKOS_INLINE_FUNCTION auto interpolate(const TIn& in, const TTuple& position, TIndices... indices) const
  {
    const auto i = floor<Index>(position.front());
    const auto d = position.front() - i;

    if constexpr (TTuple::size() == 1) {
      const auto p = in(indices..., i);
      const auto n = in(indices..., i + 1);
      return d * (n - p) + p;
    } else {
      const auto p = interpolate(in, position.tail(), indices..., i);
      const auto n = interpolate(in, position.tail(), indices..., i + 1);
      return d * (n - p) + p;
    }
  }
};

/**
 * @ingroup resampling
 * @brief Cubic interpolation.
 */
struct Cubic {
  KOKKOS_INLINE_FUNCTION auto operator()(const auto& in, std::floating_point auto... xs) const
  {
    return interpolate(in, forward_as_tuple(xs...));
  }

private:

  /**
   * @brief Return the interpolated value at given index.
   */
  template <typename TIn, typename TTuple, typename... TIndices>
  KOKKOS_INLINE_FUNCTION auto interpolate(const TIn& in, const TTuple& position, TIndices... indices) const
  {
    const auto i = floor<Index>(position.front());
    const auto d = position.front() - i;

    if constexpr (TTuple::size() == 1) {
      const auto pp = in(indices..., i - 1);
      const auto p = in(indices..., i);
      const auto n = in(indices..., i + 1);
      const auto nn = in(indices..., i + 2);
      return p + 0.5 * (d * (-pp + n) + d * d * (2 * pp - 5 * p + 4 * n - nn) + d * d * d * (-pp + 3 * p - 3 * n + nn));
    } else {
      const auto pp = interpolate(in, position.tail(), indices..., i - 1);
      const auto p = interpolate(in, position.tail(), indices..., i);
      const auto n = interpolate(in, position.tail(), indices..., i + 1);
      const auto nn = interpolate(in, position.tail(), indices..., i + 2);
      return p + 0.5 * (d * (-pp + n) + d * d * (2 * pp - 5 * p + 4 * n - nn) + d * d * d * (-pp + 3 * p - 3 * n + nn));
    }
  }
};

/**
 * @brief Grow the size of an array by a given factor.
 */
// FIXME template <typename TMethod = Forward>
class Upsample {
public:

  Upsample(Index factor) : m_factor(factor) {}

  std::string label() const
  {
    return "Upsample";
  }

  template <typename TIn>
  auto operator()(const TIn& in) const
  {
    // FIXME in.shape() not compatible for Sequence
    auto apply = lazy(in);
    TIn out(compose_label(label(), in), in.shape() * m_factor); // Differs from FilterMixin
    apply.copy_to(out);
    return out;
  }

  template <typename TIn, typename TOut>
  void transform(const TIn& in, const TOut& out) const
  {
    lazy(in).copy_to(out);
  }

  auto lazy(const auto& in) const
  {
    return Lazy(m_factor, in);
  }

  template <typename TIn>
  class Lazy {
  public:

    Lazy(Index factor, const TIn& in) : m_factor(factor), m_in(in) {}

    auto domain() const
    {
      return m_in.domain() * m_factor; // Differs from FilterMixin
    }

    KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... is) const
    {
      // FIXME if constexpr (std::is_same_v<TMethod, Forward>)
      return m_in(is / m_factor...);
    }

    template <typename TOut>
    void copy_to(TOut& out) const
    {
      for_each("copy_to", domain(), Copy(*this, out)); // TODO make generic
    }

  private:

    Index m_factor;
    TIn m_in;
  };

private:

  Index m_factor;
};

/**
 * @brief Shrink the size of an array by a given factor.
 */
// FIXME template <typename TMethod = Forward>
class Downsample { // FIXME avoid duplication
public:

  Downsample(Index factor) : m_factor(factor) {}

  std::string label() const
  {
    return "Downsample";
  }

  template <typename TIn>
  auto operator()(const TIn& in) const
  {
    auto apply = lazy(in);
    TIn out(compose_label(label(), in), apply.domain().shape());
    apply.copy_to(out);
    return out;
  }

  template <typename TIn, typename TOut>
  void transform(const TIn& in, const TOut& out) const
  {
    lazy(in).copy_to(out);
  }

  auto lazy(const auto& in) const
  {
    return Lazy(m_factor, in);
  }

  template <typename TIn>
  class Lazy {
  public:

    Lazy(Index factor, const TIn& in) : m_factor(factor), m_in(in) {}

    auto domain() const
    {
      // FIXME if constexpr (std::is_same_v<TMethod, Forward>)
      return m_in.domain() / m_factor;
    }

    KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... is) const
    {
      return m_in(is * m_factor...);
    }

    template <typename TOut>
    void copy_to(TOut& out) const
    {
      for_each("copy_to", domain(), Copy(*this, out)); // TODO make generic
    }

  private:

    Index m_factor;
    TIn m_in;
  };

private:

  Index m_factor;
};

} // namespace Linx

#endif
