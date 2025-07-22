// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_VECTOR_H
#define LINX_BASE_VECTOR_H

#include "Linx/Base/Rank.h"

#include <array>
#include <concepts>
#include <iostream>
#include <utility> // integer_sequence
#include <vector>

namespace Linx {

/**
 * @brief Container adaptor for `Vector`.
 * @tparam T Coefficients specification
 */
template <typename T>
class VectorBase;

/**
 * @brief Dynamic-rank specialization.
 * @tparam T The coefficient type
 */
template <typename T>
class VectorBase<T*> {
public:

  static constexpr int n = -1; ///< The size parameter
  using value_type = T; ///< The coefficient type
  using Container = std::vector<T>; ///< The underlying container type

  /**
   * @brief Constructor.
   */
  constexpr VectorBase(std::initializer_list<T> coefs) : VectorBase(coefs.begin(), coefs.end()) {}

  /**
   * @brief Constructor.
   */
  constexpr VectorBase(auto begin, auto end) : m_container(begin, end) {}

  /**
   * @brief Size.
   */
  constexpr auto size() const
  {
    return m_container.size();
  }

  /**
   * @brief Access the i-th coefficient.
   */
  constexpr const value_type& operator[](std::integral auto i) const
  {
    return m_container[i];
  }

private:

  Container m_container; ///< The underlying container
};

/**
 * @brief Static-size specialization.
 * @tparam T The coefficient type
 * @tparam N The size
 */
template <typename T, int N>
class VectorBase<T[N]> {
public:

  static constexpr int n = N; ///< The size parameter
  using value_type = T; ///< The coefficient type
  using Container = std::array<T, N>; ///< The underlying container type

  /**
   * @brief Constructor.
   */
  constexpr VectorBase(std::initializer_list<T> coefs) : VectorBase(coefs.begin(), coefs.end()) {}

  /**
   * @brief Constructor.
   */
  constexpr VectorBase(auto begin, auto end) : m_container {}
  {
    std::copy(begin, end, m_container.data());
  }

  /**
   * @brief Size.
   */
  static constexpr auto size()
  {
    return n;
  }

  /**
   * @brief Access the i-th coefficient.
   */
  constexpr const value_type& operator[](std::integral auto i) const
  {
    return m_container[i];
  }

private:

  Container m_container; ///< The underlying container
};

/**
 * @brief Static-coefficients specialization
 * @tparam T The coefficient type
 * @tparam Coefs The coefficients
 */
template <typename T, auto... Coefs>
class VectorBase<std::integer_sequence<T, Coefs...>> {
public:

  static constexpr int n = sizeof...(Coefs); ///< The size
  using value_type = const T; ///< The coefficient type
  using Container = void; ///< The underlying container type

  /**
   * @brief No-op constructor.
   */
  constexpr VectorBase(auto&&...) {}

  /**
   * @brief The size.
   */
  static constexpr auto size()
  {
    return n;
  }

  /**
   * @brief I-th coefficient.
   */
  constexpr value_type operator[](std::integral auto i) const
  {
    return at_impl<Coefs...>(i);
  }

private:

  /**
   * @brief Helper function to loop over coefficients.
   */
  template <auto I0 = 0, auto... Is>
  static constexpr value_type at_impl(std::integral auto i)
  {
    return i == 0 ? I0 : at_impl<Is...>(i - 1);
  }
};

/**
 * @brief Vector zero specialization.
 */
template <>
class VectorBase<void> {
public:

  static constexpr int n = 0; ///< The size
  using value_type = const int; ///< The value type
  using Container = void; ///< The underlying container

  /**
   * @brief No-op constructor.
   */
  constexpr VectorBase(auto&&...) {}

  /**
   * @brief Size. 
   */
  static constexpr int size()
  {
    return 0;
  }

  /**
   * @brief Return 0.
   */
  constexpr int operator[](auto&&) const
  {
    return at(0);
  }

  /**
   * @brief Return 0.
   */
  static constexpr int at(auto&&)
  {
    return 0;
  }
};

/**
 * @brief Non-resizable, rank-1 container on host.
 * 
 * @tparam T 
 */
template <typename T = void>
class Vector : public VectorBase<T> {
public:

  using reference = const VectorBase<T>::value_type&; ///< The reference type

  /**
   * @brief Constructor.
   */
  using VectorBase<T>::VectorBase;

  /**
   * @brief Access the i-th coefficient.
   */
  constexpr decltype(auto) operator()(std::integral auto i) const
  {
    return this->operator[](i);
  }
};

/**
 * @brief Stream insertion.
 */
template <typename T>
std::ostream& operator<<(std::ostream& os, const Vector<T>& p)
{
  if (p.size() == 0) {
    return os << "O";
  }

  os << "[" << p(0);
  for (int i = 1; i < p.size(); ++i) {
    os << ", " << p(i);
  }
  return os << "]";
}

/**
 * @brief Create a dynamic-size vector.
 */
template <typename T>
constexpr auto vec(std::initializer_list<T> coefs)
{
  return Vector<T*>(coefs.begin(), coefs.end());
}

/**
 * @brief Create a static-size vector.
 */
template <typename T, std::size_t N>
constexpr auto vec(const std::array<T, N>& coefs)
{
  return Vector<T[N]>(std::begin(coefs), std::end(coefs));
}

/**
 * @brief Create a static-size vector.
 */
template <std::integral T0, std::integral... Ts>
constexpr auto vec(T0 coef0, Ts... coefs)
{
  return vec(std::array {coef0, T0 {coefs}...});
}

/**
 * @brief Create a static-coefficients vector.
 */
template <std::integral auto Coef0, std::integral auto... Coefs>
static constexpr auto vec()
{
  using T = decltype(Coef0);
  return Vector<std::integer_sequence<T, Coef0, Coefs...>>();
}

namespace Impl {

template <typename T, auto... Is>
constexpr auto vec_impl(auto coef, std::integer_sequence<T, Is...>)
{
  return vec((Is, coef)...);
}

template <std::integral auto Coef, typename T, auto... Is>
static constexpr auto vec_impl(std::integer_sequence<T, Is...>)
{
  return vec<(Is, Coef)...>();
}

} // namespace Impl

/**
 * @brief Fill a static-size vector of given rank with a given value.
 * @tparam R The rank
 * @param coef The coefficient
 */
template <Rank R>
constexpr auto vec(auto coef)
{
  using T = decltype(coef);
  return Impl::vec_impl(coef, std::make_integer_sequence<T, R.n>());
}

/**
 * @brief Fill a static-coefficients vector of given rank with a given value.
 * @tparam R The rank
 * @tparam Coef The coefficient
 */
template <Rank R, std::integral auto Coef = 0>
static constexpr auto vec()
{
  using T = decltype(Coef);
  return Impl::vec_impl<Coef>(std::make_integer_sequence<T, R.n>());
}

/**
 * @brief Vector 0.
 */
static constexpr auto vec()
{
  return Vector<>();
}

} // namespace Linx

#endif