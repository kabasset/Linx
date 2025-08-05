// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_VECTORBASE_H
#define LINX_DATA_VECTORBASE_H

#include "Linx/Base/Dimension.h"
#include "Linx/Base/Functional.h"

#include <array>
#include <concepts>
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
  static constexpr bool static_zero_flag = false; ///< Statically zero flag

  using size_type = std::size_t; ///< The size type
  using ssize_type = std::ptrdiff_t; ///< The signed size type
  using value_type = T; ///< The value type
  using element_type = std::remove_cvref_t<value_type>; ///< The element type
  using reference = value_type&; ///< The reference type
  using const_reference = const value_type&; ///< The readonly reference type
  using pointer = value_type*; ///< The pointer type
  using const_pointer = const value_type*; /// The readonly pointer type
  using Container = std::vector<value_type>; ///< The underlying container type
  using iterator = typename Container::iterator; ///< The iterator type
  using const_iterator = typename Container::const_iterator; ///< The readonly iterator type

  /**
   * @brief Constructor.
   */
  constexpr VectorBase(std::size_t size = 0) : m_container(size) {}

  /**
   * @brief Constructor.
   */
  template <typename TCoef>
  constexpr VectorBase(std::initializer_list<TCoef> coefs) : VectorBase(coefs.begin(), coefs.end())
  {}

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
  constexpr size_type size() const
  {
    return m_container.size();
  }

  /**
   * @brief Signed size.
   */
  constexpr ssize_type ssize() const
  {
    return m_container.ssize();
  }

  /**
   * @brief Data pointer.
   */
  constexpr const_pointer data() const
  {
    return m_container.data();
  }

  /**
   * @brief Data pointer.
   */
  constexpr pointer data()
  {
    return m_container.data();
  }

  /**
   * @brief Const iterator to the beginning.
   */
  constexpr const_iterator begin() const
  {
    return m_container.begin();
  }

  /**
   * @brief Iterator to the beginning.
   */
  constexpr iterator begin()
  {
    return m_container.begin();
  }

  /**
   * @brief Const iterator to the end.
   */
  constexpr const_iterator end() const
  {
    return m_container.end();
  }

  /**
   * @brief Iterator to the end.
   */
  constexpr iterator end()
  {
    return m_container.end();
  }

  /**
   * @brief Access the i-th coefficient.
   */
  constexpr const_reference operator[](std::integral auto i) const
  {
    return m_container[i];
  }

  /**
   * @brief Access the i-th coefficient.
   */
  constexpr reference operator[](std::integral auto i)
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
  static constexpr bool static_zero_flag = (n == 0); ///< Statically zero flag

  using size_type = std::size_t; ///< The size type
  using ssize_type = std::ptrdiff_t; ///< The signed size type
  using value_type = T; ///< The value type
  using element_type = std::remove_cvref_t<value_type>; ///< The element type
  using reference = value_type&; ///< The reference type
  using const_reference = const value_type&; ///< The readonly reference type
  using pointer = value_type*; ///< The pointer type
  using const_pointer = const value_type*; /// The readonly pointer type
  using Container = std::array<T, N>; ///< The underlying container type
  using iterator = typename Container::iterator; ///< The iterator type
  using const_iterator = typename Container::const_iterator; ///< The readonly iterator type

  /**
   * @brief Constructor.
   */
  constexpr VectorBase() : m_container {} {}

  /**
   * @brief Constructor.
   */
  template <typename TCoef>
  constexpr VectorBase(std::initializer_list<TCoef> coefs) : VectorBase(coefs.begin(), coefs.end())
  {}

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
  static constexpr size_type size()
  {
    return N;
  }

  /**
   * @brief Signed size.
   */
  static constexpr ssize_type ssize()
  {
    return n;
  }

  /**
   * @brief Data pointer.
   */
  constexpr const_pointer data() const
  {
    return m_container.data();
  }

  /**
   * @brief Data pointer.
   */
  constexpr pointer data()
  {
    return m_container.data();
  }

  /**
   * @brief Const iterator to the beginning.
   */
  constexpr const_iterator begin() const
  {
    return m_container.begin();
  }

  /**
   * @brief Iterator to the beginning.
   */
  constexpr iterator begin()
  {
    return m_container.begin();
  }

  /**
   * @brief Const iterator to the end.
   */
  constexpr const_iterator end() const
  {
    return m_container.end();
  }

  /**
   * @brief Iterator to the end.
   */
  constexpr iterator end()
  {
    return m_container.end();
  }

  /**
   * @brief Access the i-th coefficient.
   */
  constexpr const_reference operator[](std::integral auto i) const
  {
    return m_container[i];
  }

  /**
   * @brief Access the i-th coefficient.
   */
  constexpr reference operator[](std::integral auto i)
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
  static constexpr bool static_zero_flag = ((Coefs == 0) && ...); ///< Statically zero flag

  using size_type = std::size_t; ///< The size type
  using ssize_type = std::ptrdiff_t; ///< The signed size type
  using value_type = const T; ///< The value type
  using element_type = std::remove_cvref_t<value_type>; ///< The element type
  using reference = value_type&; ///< The reference type
  using const_reference = const value_type&; ///< The readonly reference type
  using pointer = value_type*; ///< The pointer type
  using const_pointer = const value_type*; /// The readonly pointer type
  using Container = void; ///< The underlying container type

  /**
   * @brief No-op constructor.
   */
  constexpr VectorBase(auto&&...) {}

  /**
   * @brief Size.
   */
  static constexpr size_type size()
  {
    return sizeof...(Coefs);
  }

  /**
   * @brief Signed size.
   */
  static constexpr ssize_type ssize()
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
