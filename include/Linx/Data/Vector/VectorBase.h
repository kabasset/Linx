// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_VECTORBASE_H
#define LINX_DATA_VECTORBASE_H

#include "Linx/Base/Dimension.h"
#include "Linx/Base/Functional.h"

#include <array>
#include <concepts>
#include <iostream>
#include <utility> // integer_sequence
#include <vector>

namespace Linx {

/**
 * @brief Container adaptor for `Vector`.
 * @tparam T The coefficients specification
 * 
 * Only specializations are defined.
 * They must implement the `LegacyArray` interface.
 */
template <typename T>
class VectorBase;

template <typename T>
class Vector;

/**
 * @brief Dynamic-rank specialization.
 * @tparam T The coefficient type
 */
template <typename T>
class VectorBase<T*> {
public:

  static constexpr int n = -1; ///< The size parameter
  using value_type = T; ///< The value type
  using Container = std::vector<T>; ///< The underlying container type

  /**
   * @brief Constructor.
   */
  constexpr VectorBase(std::size_t size = 0) : m_container(size) {}

  /**
   * @brief Constructor.
   */
  constexpr VectorBase(std::initializer_list<T> coefs) : VectorBase(coefs.begin(), coefs.end()) {}

  /**
   * @brief Constructor.
   */
  constexpr VectorBase(auto begin, auto end) : m_container(begin, end) {}

  /**
   * @brief Forwarding constructor.
   */
  constexpr VectorBase(Forward&&, auto&& arg0, auto&&... args) : m_container(LINX_FORWARD(arg0), LINX_FORWARD(args)...)
  {}

  /**
   * @brief Conversion constructor.
   */
  template <typename TSpec>
  constexpr VectorBase(const Vector<TSpec>& other) : VectorBase(other.size())
  {
    for (std::size_t i = 0; i < size(); ++i) {
      m_container[i] = other[i];
    }
  }

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

  /**
   * @brief Access the i-th coefficient.
   */
  constexpr value_type& operator[](std::integral auto i)
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
  using value_type = T; ///< The value type
  using Container = std::array<T, N>; ///< The underlying container type

  /**
   * @brief Constructor.
   */
  constexpr VectorBase() : m_container {} {}

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
   * @brief Forwarding constructor.
   */
  constexpr VectorBase(Forward&&, auto&& arg0, auto&&... args) : m_container(LINX_FORWARD(arg0), LINX_FORWARD(args)...)
  {}

  /**
   * @brief Conversion constructor.
   */
  template <typename TSpec>
  constexpr VectorBase(const Vector<TSpec>& other) : VectorBase()
  {
    for (std::size_t i = 0; i < size(); ++i) {
      m_container[i] = other[i];
    }
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

  /**
   * @brief Access the i-th coefficient.
   */
  constexpr value_type& operator[](std::integral auto i)
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
  using value_type = const T; ///< The value type
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
    return at(i);
  }

  /**
   * @brief I-th coefficient.
   */
  static constexpr value_type at(std::integral auto i)
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
 * @brief I-th coefficient of a static vector.
 */
template <std::integral auto I, typename T, std::integral auto... Is>
static constexpr auto get(const VectorBase<std::integer_sequence<T, Is...>>&)
{
  return VectorBase<std::integer_sequence<T, Is...>>::at(I);
}

/**
 * @brief I-th coefficient of a static vector, or a fallback value if i is out of bounds.
 */
template <auto I, auto Fallback, typename T, std::integral auto... Is>
static constexpr auto get_or(const VectorBase<std::integer_sequence<T, Is...>>&)
{
  return I < 0 || I >= sizeof...(Is) ? Fallback : get<I>(VectorBase<std::integer_sequence<T, Is...>>());
}

template <auto I, auto Fallback, typename T>
constexpr auto get_or(const VectorBase<T>& v)
{
  return I < 0 || I >= v.size() ? Fallback : v[I];
}

} // namespace Linx

#endif
