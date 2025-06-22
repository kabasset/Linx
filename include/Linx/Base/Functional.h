// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_FUNCTIONAL_H
#define LINX_BASE_FUNCTIONAL_H

#include "Linx/Base/Types.h"

#include <Kokkos_Core.hpp>

namespace Linx {

/**
 * @brief Functor which forwards its argument.
 */
struct Forward {
  std::string label() const
  {
    return "Forward";
  }

  KOKKOS_INLINE_FUNCTION constexpr decltype(auto) operator()(auto&& value) const
  {
    return LINX_FORWARD(value);
  }
};

/**
 * @brief Element-wise copy. 
 */
template <typename TIn, typename TOut>
struct Copy {
  TIn m_in; // FIXME private
  TOut m_out;
  KOKKOS_INLINE_FUNCTION void operator()(std::integral auto... is) const
  {
    m_out(is...) = m_in(is...);
  }
};

/**
 * @brief Logical not.
 */
struct Not {
  std::string label() const
  {
    return "Not";
  }

  KOKKOS_INLINE_FUNCTION constexpr bool operator()(const auto& value) const
  {
    return not value;
  }
};

/**
 * @brief Negation.
 */
struct Negate {
  std::string label() const
  {
    return "Negate";
  }

  KOKKOS_INLINE_FUNCTION auto operator()(const auto& e) const
  {
    return -e;
  }
};

/**
 * @brief Functor which always returns the same value.
 */
template <typename T>
struct Constant {
  using value_type = const T; ///< The value type

  const T value; ///< The value

  /**
   * @brief Constructor.
   */
  KOKKOS_INLINE_FUNCTION explicit Constant(T v) : value {LINX_MOVE(v)} {}

  /**
   * @brief Label.
   */
  std::string label() const
  {
    return compose_label("Constant", value);
  }

  /**
   * @brief Reference to the value.
   */
  KOKKOS_INLINE_FUNCTION constexpr const value_type& operator()(auto&&...) const
  {
    return value;
  }
};

/**
 * @brief Functor which always returns the same value set at compile time.
 */
template <auto Value>
struct StaticConstant {
  using value_type = decltype(Value);

  static constexpr value_type value = Value; ///< The static value

  /**
   * @brief Constructor.
   */
  KOKKOS_INLINE_FUNCTION StaticConstant(auto&&...) {}

  /**
   * @brief Label.
   */
  std::string label() const
  {
    return compose_label("Constant", value);
  }

  /**
   * @brief Static value.
   */
  KOKKOS_INLINE_FUNCTION constexpr value_type operator()(auto&&...) const
  {
    return value;
  }
};

/**
 * @brief Functor which tests whether a value is between inclusive or exclusive endpoints.
 */
template <bool InclusiveInfimum, bool InclusiveSupremum, typename T>
struct Between {
  using value_type = T; ///< The value type

  /**
   * @brief Always-false functor constructor.
   */
  KOKKOS_INLINE_FUNCTION Between() : infimum(Limits<T>::max()), supremum(Limits<T>::min()) {} // FIXME rm

  /**
   * @brief Constructor.
   */
  KOKKOS_INLINE_FUNCTION Between(const T& inf, const T& sup) : infimum(inf), supremum(sup) {}

  /**
   * @brief Size-based constructor.
   */
  template <typename TSize>
  KOKKOS_INLINE_FUNCTION Between(const T& inf, const Size<TSize>& size) : infimum(inf), supremum(infimum + size.value)
  {
    if constexpr (std::is_integral_v<T>) {
      supremum += InclusiveInfimum + InclusiveSupremum - 1;
    }
  };

  /**
   * @brief Check whether a value is between the endpoints.
   */
  KOKKOS_INLINE_FUNCTION bool operator()(const T& value) const
  {
    return greater_than_infimum(value) && less_than_supremum(value);
  }

  /**
   * @brief Check whether a value is greater than the infimum.
   */
  KOKKOS_INLINE_FUNCTION bool greater_than_infimum(const T& value) const
  {
    if constexpr (InclusiveInfimum) {
      return value >= infimum;
    } else {
      return value > infimum;
    }
  }

  /**
   * @brief Check whether a value is less than the supremum.
   */
  KOKKOS_INLINE_FUNCTION bool less_than_supremum(const T& value) const
  {
    if constexpr (InclusiveSupremum) {
      return value <= supremum;
    } else {
      return value < supremum;
    }
  }

  T infimum; ///< The interval infimum
  T supremum; ///< The interval supremum
};

#define LINX_DEFINE_BINARY_OPERATOR(Func, out) \
  template <typename TLhs = Forward, typename TRhs = Forward> \
  struct Func; \
\
  template <typename TRhs> \
  struct Func<Forward, TRhs> { \
    TRhs rhs; \
    KOKKOS_INLINE_FUNCTION Func(TRhs value) : rhs {value} {} \
    KOKKOS_INLINE_FUNCTION auto operator()(const auto& lhs) const \
    { \
      return out; \
    } \
  }; \
\
  template <typename TLhs> \
  struct Func<TLhs, Forward> { \
    TLhs lhs; \
    KOKKOS_INLINE_FUNCTION Func(TLhs value) : lhs {value} {} \
    KOKKOS_INLINE_FUNCTION auto operator()(const auto& rhs) const \
    { \
      return out; \
    } \
  }; \
\
  template <> \
  struct Func<Forward, Forward> { \
    KOKKOS_INLINE_FUNCTION auto operator()(const auto& lhs, const auto& rhs) const \
    { \
      return out; \
    } \
  }; \
\
  Func() -> Func<Forward, Forward>; \
  template <typename T> \
  Func(T) -> Func<Forward, T>;

#define LINX_DEFINE_MONOID(Func, out, identity) \
  LINX_DEFINE_BINARY_OPERATOR(Func, out) \
\
  template <typename T, typename TLhs, typename TRhs> \
  KOKKOS_INLINE_FUNCTION auto identity_element(const Func<TLhs, TRhs>&) \
  { \
    return identity; \
  }

LINX_DEFINE_MONOID(Add, (lhs + rhs), T {})
LINX_DEFINE_BINARY_OPERATOR(Subtract, (lhs - rhs))
LINX_DEFINE_MONOID(Multiply, (lhs * rhs), T {1})
LINX_DEFINE_BINARY_OPERATOR(Divide, (lhs / rhs))
LINX_DEFINE_BINARY_OPERATOR(Modulus, (lhs % rhs))
LINX_DEFINE_BINARY_OPERATOR(Equal, (lhs == rhs))
LINX_DEFINE_BINARY_OPERATOR(NotEqual, (lhs != rhs))
LINX_DEFINE_MONOID(And, (lhs && rhs), true)
LINX_DEFINE_MONOID(Or, (lhs || rhs), false)
LINX_DEFINE_BINARY_OPERATOR(Xor, (!lhs != !rhs))
LINX_DEFINE_MONOID(Min, std::min(lhs, rhs), std::numeric_limits<T>::max())
LINX_DEFINE_MONOID(Max, std::max(lhs, rhs), std::numeric_limits<T>::lowest())

#undef LINX_DEFINE_BINARY_OPERATOR
#undef LINX_DEFINE_MONOID

/**
 * @brief Compute the absolute value of an integral power.
 * @see `Abspow`
 */
template <int P, typename T>
KOKKOS_INLINE_FUNCTION constexpr T abspow(T x)
{
  if constexpr (P == 0) {
    return bool(x);
  }
  if constexpr (P == 1) {
    return x >= 0 ? x : -x;
  }
  if constexpr (P == 2) {
    return x * x;
  }
  if constexpr (P > 2) {
    return x * x * abspow<P - 2>(x);
  }
}

/**
 * @brief Functor which returns `abspow()`.
 */
template <int P>
struct Abspow {
  KOKKOS_INLINE_FUNCTION constexpr auto operator()(const auto& lhs) const
  {
    return abspow<P>(lhs);
  }
  KOKKOS_INLINE_FUNCTION constexpr auto operator()(const auto& lhs, const auto& rhs) const
  {
    return abspow<P>(rhs - lhs);
  }
};

/**
 * @brief Functor which returns `true` iff `value != value`.
 */
struct IsNan {
  KOKKOS_INLINE_FUNCTION constexpr bool operator()(const auto& value) const
  {
    return value != value;
  }
};

namespace Impl {
template <typename TFunc, typename T, typename Is, typename = void>
struct can_accept_impl : std::false_type {};

template <typename TFunc, typename T, std::size_t... Is>
struct can_accept_impl<
    TFunc,
    T,
    std::index_sequence<Is...>,
    decltype(std::declval<TFunc>()(((void)Is, T())...), void())> : std::true_type {};
} // namespace Impl

template <typename TFunc, typename T, int N>
constexpr bool is_nadic()
{
  return Impl::can_accept_impl<TFunc, T, std::make_index_sequence<N>>::value;
}

namespace Impl {

template <typename... TFuncs>
struct Compose;

template <typename TFunc>
struct Compose<TFunc> {
  TFunc m_f;
  KOKKOS_INLINE_FUNCTION Compose(TFunc f) : m_f(f) {}
  KOKKOS_INLINE_FUNCTION auto operator()(auto&&... args) const
  {
    return m_f(LINX_FORWARD(args)...);
  }
};

template <typename TFunc0, typename... TFuncs>
struct Compose<TFunc0, TFuncs...> {
  TFunc0 m_f0;
  Compose<TFuncs...> m_fs;
  KOKKOS_INLINE_FUNCTION Compose(TFunc0 f0, TFuncs... fs) : m_f0(f0), m_fs(fs...) {};
  KOKKOS_INLINE_FUNCTION auto operator()(auto&&... args) const
  {
    return m_fs(m_f0(LINX_FORWARD(args)...));
  }
};

} // namespace Impl

/**
 * @brief Functional composition.
 * 
 * This function returns a function such that:
 * `compose_function(f, g, h)(args...)` returns `h(g(f(args...)))`.
 */
template <typename TFunc0, typename... TFuncs>
auto compose_functions(TFunc0 f0, TFuncs... fs)
{
  return Impl::Compose<TFunc0, TFuncs...>(f0, fs...);
}

} // namespace Linx

#endif
