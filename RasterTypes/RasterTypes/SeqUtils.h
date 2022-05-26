// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERTYPES_SEQUTILS_H
#define _RASTERTYPES_SEQUTILS_H

#include <complex> // FIXME from TypeUtils?
#include <tuple>
#include <utility> // declval

namespace Cnes {

/**
 * @brief List of supported integral types.
 */
#define CNES_RASTER_SUPPORTED_INTS \
  bool, unsigned char, char, signed char, unsigned short, signed short, unsigned int, signed int, unsigned long, \
      signed long, unsigned long long, signed long long

/**
 * @brief List of supported floating point types.
 */
#define CNES_RASTER_SUPPORTED_FLOATS float, double, long double

/**
 * @brief List of supported complex types.
 */
#define CNES_RASTER_SUPPORTED_COMPLEXES std::complex<float>, std::complex<double>, std::complex<long double>

/**
 * @brief List of supported types.
 */
#define CNES_RASTER_SUPPORTED_TYPES \
  CNES_RASTER_SUPPORTED_INTS, CNES_RASTER_SUPPORTED_FLOATS, CNES_RASTER_SUPPORTED_COMPLEXES

/**
 * @brief List of supported types as a tuple.
 */
using RasterSupportedTypesTuple = std::tuple<CNES_RASTER_SUPPORTED_TYPES>;

/**
 * @brief `BOOST_AUTO_TEST_CASE_TEMPLATE` for each supported type.
 */
#define CNES_RASTER_TEST_CASE_TEMPLATE(name) BOOST_AUTO_TEST_CASE_TEMPLATE(name, T, RasterSupportedTypesTuple)

/**
 * @brief Utility type for SFINAE, equivalent to C++17's `std::void_t`.
 */
template <typename...>
using templateVoid = void; // FIXME to TypeUtils

/**
 * @brief Test whether a type is iterable, i.e. has `begin()` and `end()` methods.
 */
template <typename T, typename = void>
struct isIterable : std::false_type {};

/// @cond
// https://en.cppreference.com/w/cpp/types/void_t
template <typename T>
struct isIterable<T, templateVoid<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>> :
    std::true_type {};
/// @endcond

} // namespace Cnes

#endif // _RASTERTYPES_SEQUTILS_H
