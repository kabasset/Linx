// Copyright (C) 2022, CNES
// This file is part of KRaster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _KRASTER_TESTUTILS_H
#define _KRASTER_TESTUTILS_H

#include <complex>
#include <vector>

namespace Cnes {

/**
 * @brief Test-related classes and functions.
 */
namespace Test {

/**
 * @brief Value very close to the min of the type.
 */
template <typename T>
T almostMin();

/**
 * @brief Value very close to the max of the type.
 */
template <typename T>
T almostMax();

/**
 * @brief Half the min of the type.
 */
template <typename T>
T halfMin();

/**
 * @brief Half the max of the type.
 */
template <typename T>
T halfMax();

/**
 * @brief Generate a random value of given type.
 */
template <typename T>
T generateRandomValue(T min = halfMin<T>(), T max = halfMax<T>());

/**
 * @brief Generate a random vector of given type and size.
 */
template <typename T>
std::vector<T> generateRandomVector(std::size_t size, T min = halfMin<T>(), T max = halfMax<T>());

/**
 * @brief Check whether a test value is approximately equal to a reference value.
 * @details
 * Floating point values are compared as: |test - ref| / ref < tol
 * Complex values are tested component-wise.
 * Other types are tested for equality.
 */
template <typename T>
bool approx(T test, T ref, double tol = 0.01);

} // namespace Test
} // namespace Cnes

/// @cond
#define _KRASTER_TESTUTILS_IMPL
#include "KRaster/impl/TestUtils.hpp"
#undef _KRASTER_TESTUTILS_IMPL
/// @endcond

#endif // _KRASTER_TESTUTILS_H
