// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXBASE_SEQUTILS_H
#define _LINXBASE_SEQUTILS_H

#include "Linx/Base/TypeUtils.h"

#include <algorithm> // transform
#include <array>
#include <complex>
#include <string>
#include <tuple>
#include <type_traits> // remove_reference, true_type, false_type
#include <utility> // declval, forward, index_sequence
#include <vector>

namespace Linx {

/**
 * @brief Test whether a type is a range, i.e. has `begin()` and `end()` methods.
 */
template <typename T, typename = void>
struct IsRange : std::false_type {};

/// @cond
// https://en.cppreference.com/w/cpp/types/void_t
template <typename T>
struct IsRange<T, std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>> :
    std::true_type {}; // FIXME std::begin(T), std::end(T)
/// @endcond

/// @cond
namespace Internal {

/**
 * @brief Make an index sequence for a tuple.
 */
template <typename TTuple>
constexpr decltype(auto) tuple_index_sequence()
{
  return std::make_index_sequence<std::tuple_size<std::remove_reference_t<TTuple>>::value>();
}

/**
 * @brief Convert a tuple to a user-defined struct.
 */
template <typename TReturn, typename TTuple, std::size_t... Is>
TReturn tuple_as_impl(TTuple&& tuple, std::index_sequence<Is...>)
{
  return {std::get<Is>(tuple)...};
}

/**
 * @brief Apply a variadic function to a tuple.
 */
template <typename TTuple, typename TFunc, std::size_t... Is>
constexpr decltype(auto) apply_impl(TTuple&& tuple, TFunc&& func, std::index_sequence<Is...>)
{
  return func(std::get<Is>(tuple)...);
}

/**
 * @brief Apply a variadic function to elements pointed by a tuple of iterators
 * and increment the iterators.
 */
template <typename TIteratorTuple, typename TFunc, std::size_t... Is>
constexpr decltype(auto) iterator_tuple_apply_impl(TIteratorTuple&& tuple, TFunc&& func, std::index_sequence<Is...>)
{
  return func(*std::get<Is>(tuple)++...);
}

/**
 * @brief Apply a function which returns void to each element of a tuple.
 */
template <typename TTuple, typename TFunc, std::size_t... Is>
void tuple_foreach_impl(TTuple&& tuple, TFunc&& func, std::index_sequence<Is...>)
{
  using mock_unpack = int[];
  (void)mock_unpack {
      0, // Ensure there is at least one element
      (func(std::get<Is>(tuple)), // Use comma operator to return an int even if func doesn't
       void(), // Add void() in case where the return type of func would define a comma-operator
       0)...};
}

/**
 * @brief Apply a function to each element of a tuple, and make a user-defined struct from the results.
 */
template <typename TReturn, typename TTuple, typename TFunc, std::size_t... Is>
TReturn tuple_transform_impl(TTuple&& tuple, TFunc&& func, std::index_sequence<Is...>)
{
  return {func(std::get<Is>(tuple))...};
}

/**
 * @brief Traits class to test wether a sequence is a tuple.
 * 
 * Use `IsTupleImpl<T>::value` to get a `bool`.
 */
template <typename TSeq>
struct IsTupleImpl : std::false_type {};

/**
 * @copydoc IsTupleImpl
 */
template <typename... Ts>
struct IsTupleImpl<std::tuple<Ts...>> : std::true_type {};

/**
 * @copydoc IsTupleImpl
 */
template <typename T, std::size_t N>
struct IsTupleImpl<std::array<T, N>> : std::true_type {};

} // namespace Internal
/// @endcond

/**
 * @brief Test whether a sequence is a tuple.
 */
template <typename TSeq>
constexpr bool is_tuple()
{
  return Internal::IsTupleImpl<std::decay_t<TSeq>>::value;
}

/**
 * @brief Convert a tuple to a custom structure.
 */
template <typename TReturn, typename TTuple>
TReturn tuple_as(TTuple&& tuple)
{
  return Internal::tuple_as_impl<TReturn>(std::forward<TTuple>(tuple), Internal::tuple_index_sequence<TTuple>());
}

/**
 * @brief Apply a function to the elements of a tuple.
 */
template <typename TTuple, typename TFunc>
constexpr decltype(auto) tuple_apply(TTuple&& tuple, TFunc&& func)
{
  return Internal::apply_impl(
      std::forward<TTuple>(tuple),
      std::forward<TFunc>(func),
      Internal::tuple_index_sequence<TTuple>());
}

/**
 * @brief Apply a function to the elements pointed by a tuple of iterators
 * and then increment the iterators.
 */
template <typename TIteratorTuple, typename TFunc>
constexpr decltype(auto) iterator_tuple_apply(TIteratorTuple&& tuple, TFunc&& func)
{
  return Internal::iterator_tuple_apply_impl(
      std::forward<TIteratorTuple>(tuple),
      std::forward<TFunc>(func),
      Internal::tuple_index_sequence<TIteratorTuple>());
}

/**
 * @brief Apply a void-returning function to each element of a sequence.
 */
template <typename TSeq, typename TFunc>
std::enable_if_t<is_tuple<TSeq>()> seq_foreach(TSeq&& seq, TFunc&& func)
{
  Internal::tuple_foreach_impl(
      std::forward<TSeq>(seq),
      std::forward<TFunc>(func),
      Internal::tuple_index_sequence<TSeq>());
}

/**
 * @copydoc seq_foreach()
 */
template <typename TSeq, typename TFunc>
std::enable_if_t<not is_tuple<TSeq>()> seq_foreach(const TSeq& seq, TFunc&& func)
{
  for (const auto& element : seq) {
    func(element);
  }
}

/**
 * @copydoc seq_foreach()
 */
template <typename TSeq, typename TFunc>
std::enable_if_t<not is_tuple<TSeq>()> seq_foreach(TSeq& seq, TFunc&& func)
{
  for (auto& element : seq) {
    func(element);
  }
}

/**
 * @brief Apply a transform to each element of a sequence and create a user-defined struct from the results.
 */
template <typename TReturn, typename TSeq, typename TFunc>
std::enable_if_t<is_tuple<TSeq>(), TReturn> seq_transform(TSeq&& seq, TFunc&& func)
{
  return Internal::tuple_transform_impl<TReturn>(
      std::forward<TSeq>(seq),
      std::forward<TFunc>(func),
      Internal::tuple_index_sequence<TSeq>());
}

/**
 * @copydoc seq_transform()
 */
template <typename TReturn, typename TSeq, typename TFunc>
std::enable_if_t<not is_tuple<TSeq>(), TReturn> seq_transform(const TSeq& seq, TFunc&& func)
{
  TReturn res(seq.size());
  std::transform(seq.begin(), seq.end(), res.begin(), std::forward<TFunc>(func));
  return res;
}

/**
 * @brief Serialize a heterogeneous list of arguments.
 * 
 * Applies `operator<<()` to arguments, separated with ", ".
 * For example:
 * \code
 * serialize(std::cout, 1, 3.14, "str");
 * \endcode
 * Prints: `1, 3.14, str`
 */
template <typename TLogger, typename T0, typename... Ts>
void serialize(TLogger&& logger, T0&& arg0, Ts&&... args)
{
  logger << std::forward<T0>(arg0);
  using mock_unpack = int[];
  (void)mock_unpack {0, (void(std::forward<TLogger>(logger) << ", " << std::forward<Ts>(args)), 0)...};
}

} // namespace Linx

#endif
