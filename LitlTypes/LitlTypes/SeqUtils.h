// Copyright (C) 2022, Antoine Basset
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTYPES_SEQUTILS_H
#define _LITLTYPES_SEQUTILS_H

#include "LitlTypes/TypeUtils.h"

#include <algorithm> // transform
#include <array>
#include <complex>
#include <string>
#include <tuple>
#include <type_traits> // remove_reference, true_type, false_type
#include <utility> // declval, forward, index_sequence
#include <vector>

namespace Litl {

/**
 * @brief List of supported integral types.
 */
#define LITL_SUPPORTED_INTS \
  bool, unsigned char, char, signed char, unsigned short, signed short, unsigned int, signed int, unsigned long, \
      signed long, unsigned long long, signed long long

/**
 * @brief List of supported floating point types.
 */
#define LITL_SUPPORTED_FLOATS float, double, long double

/**
 * @brief List of supported complex types.
 */
#define LITL_SUPPORTED_COMPLEXES std::complex<float>, std::complex<double>, std::complex<long double>

/**
 * @brief List of supported types.
 */
#define LITL_SUPPORTED_TYPES LITL_SUPPORTED_INTS, LITL_SUPPORTED_FLOATS, LITL_SUPPORTED_COMPLEXES

/**
 * @brief List of supported types as a tuple.
 */
using RasterSupportedTypesTuple = std::tuple<LITL_SUPPORTED_TYPES>;

/**
 * @brief `BOOST_AUTO_TEST_CASE_TEMPLATE` for each supported type.
 */
#define LITL_TEST_CASE_TEMPLATE(name) BOOST_AUTO_TEST_CASE_TEMPLATE(name, T, RasterSupportedTypesTuple)

/**
 * @brief Test whether a type is iterable, i.e. has `begin()` and `end()` methods.
 */
template <typename T, typename = void>
struct isIterable : std::false_type {};

/// @cond
// https://en.cppreference.com/w/cpp/types/void_t
template <typename T>
struct isIterable<T, templateVoid<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>> : // FIXME std::begin(T), std::end(T)
    std::true_type {};
/// @endcond

/// @cond
namespace Internal {

/**
 * @brief Make an index sequence for a tuple.
 */
template <typename TTuple>
constexpr decltype(auto) tupleIndexSequence() {
  return std::make_index_sequence<std::tuple_size<std::remove_reference_t<TTuple>>::value>();
}

/**
 * @brief Convert a tuple to a user-defined struct.
 */
template <typename TReturn, typename TTuple, std::size_t... Is>
TReturn tupleAsImpl(TTuple&& tuple, std::index_sequence<Is...>) {
  return {std::get<Is>(tuple)...};
}

/**
 * @brief Apply a variadic function to a tuple.
 */
template <typename TTuple, typename TFunc, std::size_t... Is>
constexpr decltype(auto) applyImpl(TTuple&& tuple, TFunc&& func, std::index_sequence<Is...>) {
  return func(std::get<Is>(tuple)...);
}

/**
 * @brief Apply a variadic function to elements pointed by a tuple of iterators
 * and increment the iterators.
 */
template <typename TIteratorTuple, typename TFunc, std::size_t... Is>
constexpr decltype(auto) iteratorTupleApplyImpl(TIteratorTuple&& tuple, TFunc&& func, std::index_sequence<Is...>) {
  return func(*std::get<Is>(tuple)++...);
}

/**
 * @brief Apply a function which returns void to each element of a tuple.
 */
template <typename TTuple, typename TFunc, std::size_t... Is>
void tupleForeachImpl(TTuple&& tuple, TFunc&& func, std::index_sequence<Is...>) {
  using mockUnpack = int[];
  (void)mockUnpack {
      0, // Ensure there is at least one element
      (func(std::get<Is>(tuple)), // Use comma operator to return an int even if func doesn't
       void(), // Add void() in case where the return type of func would define a comma-operator
       0)...};
}

/**
 * @brief Apply a function to each element of a tuple, and make a user-defined struct from the results.
 */
template <typename TReturn, typename TTuple, typename TFunc, std::size_t... Is>
TReturn tupleTransformImpl(TTuple&& tuple, TFunc&& func, std::index_sequence<Is...>) {
  return {func(std::get<Is>(tuple))...};
}

/**
 * @brief Traits class to test wether a sequence is a tuple.
 * @details
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
constexpr bool isTuple() {
  return Internal::IsTupleImpl<std::decay_t<TSeq>>::value;
}

/**
 * @brief Convert a tuple to a custom structure.
 */
template <typename TReturn, typename TTuple>
TReturn tupleAs(TTuple&& tuple) {
  return Internal::tupleAsImpl<TReturn>(std::forward<TTuple>(tuple), Internal::tupleIndexSequence<TTuple>());
}

/**
 * @brief Apply a variadic function a tuple.
 */
template <typename TTuple, typename TFunc>
constexpr decltype(auto) tupleApply(TTuple&& tuple, TFunc&& func) {
  return Internal::applyImpl(
      std::forward<TTuple>(tuple),
      std::forward<TFunc>(func),
      Internal::tupleIndexSequence<TTuple>());
}

/**
 * @brief Apply a variadic function to elements pointed by a tuple of iterators
 * and increment the iterators.
 */
template <typename TIteratorTuple, typename TFunc>
constexpr decltype(auto) iteratorTupleApply(TIteratorTuple&& tuple, TFunc&& func) {
  return Internal::iteratorTupleApplyImpl(
      std::forward<TIteratorTuple>(tuple),
      std::forward<TFunc>(func),
      Internal::tupleIndexSequence<TIteratorTuple>());
}

/**
 * @brief Apply a void-returning function to each element of a sequence.
 */
template <typename TSeq, typename TFunc>
std::enable_if_t<isTuple<TSeq>()> seqForeach(TSeq&& seq, TFunc&& func) {
  Internal::tupleForeachImpl(std::forward<TSeq>(seq), std::forward<TFunc>(func), Internal::tupleIndexSequence<TSeq>());
}

/**
 * @copydoc seqForeach()
 */
template <typename TSeq, typename TFunc>
std::enable_if_t<not isTuple<TSeq>()> seqForeach(const TSeq& seq, TFunc&& func) {
  for (const auto& element : seq) {
    func(element);
  }
}

/**
 * @copydoc seqForeach()
 */
template <typename TSeq, typename TFunc>
std::enable_if_t<not isTuple<TSeq>()> seqForeach(TSeq& seq, TFunc&& func) {
  for (auto& element : seq) {
    func(element);
  }
}

/**
 * @brief Apply a transform to each element of a sequence and create a user-defined struct from the results.
 */
template <typename TReturn, typename TSeq, typename TFunc>
std::enable_if_t<isTuple<TSeq>(), TReturn> seqTransform(TSeq&& seq, TFunc&& func) {
  return Internal::tupleTransformImpl<TReturn>(
      std::forward<TSeq>(seq),
      std::forward<TFunc>(func),
      Internal::tupleIndexSequence<TSeq>());
}

/**
 * @copydoc seqTransform()
 */
template <typename TReturn, typename TSeq, typename TFunc>
std::enable_if_t<not isTuple<TSeq>(), TReturn> seqTransform(const TSeq& seq, TFunc&& func) {
  TReturn res(seq.size());
  std::transform(seq.begin(), seq.end(), res.begin(), std::forward<TFunc>(func));
  return res;
}

/**
 * @brief Serialize a heterogeneous list of arguments.
 * @details
 * Applies `operator<<()` to arguments, separated with ", ".
 * For example:
 * \code
 * serialize(std::cout, 1, 3.14, "str");
 * \endcode
 * Prints: `1, 3.14, str`
 */
template <typename TLogger, typename T0, typename... Ts>
void serialize(TLogger&& logger, T0&& arg0, Ts&&... args) {
  logger << std::forward<T0>(arg0);
  using mockUnpack = int[];
  (void)mockUnpack {0, (void(std::forward<TLogger>(logger) << ", " << std::forward<Ts>(args)), 0)...};
}

} // namespace Litl

#endif
