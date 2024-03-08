// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXBASE_DIMENSION_H
#define _LINXBASE_DIMENSION_H

#include "Linx/Base/TypeUtils.h"

#include <array>
#include <vector>

namespace Linx {

/**
 * @brief Base class for multidimensional objects.
 */
template <Index N>
struct Dimensional {
  static constexpr Index Dimension = N; ///< Compile-time dimension parameter
  static constexpr Index OneLessDimension = N - 1; ///< One less dimension
  static constexpr Index OneMoreDimension = N + 1; ///< One more dimension
  template <typename T>
  using Coordinates = std::array<T, N>; ///< Coordinates container
};

/**
 * @brief Specialization for N = 0.
 */
template <>
struct Dimensional<0> {
  static constexpr Index Dimension = 0; ///< @copydoc Dimensional::Dimension
  static constexpr Index OneLessDimension = 0; ///< @copydoc Dimensional::OneLessDimension
  static constexpr Index OneMoreDimension = 1; ///< @copydoc Dimensional::OneLessDimension
  template <typename T>
  using Coordinates = std::array<T, 0>; ///< @copydoc Dimensional::Coordinates
};

/**
 * @brief Specialization for N = -1 (runtime definition of the dimension).
 */
template <>
struct Dimensional<-1> {
  static constexpr Index Dimension = -1; ///< @copydoc Dimensional::Dimension
  static constexpr Index OneLessDimension = -1; ///< @copydoc Dimensional::OneLessDimension
  static constexpr Index OneMoreDimension = -1; ///< @copydoc Dimensional::OneMoreDimension
  template <typename T>
  using Coordinates = std::vector<T>; ///< @copydoc Dimensional::Coordinates
};

template <typename T, Index N>
using Coordinates = typename Dimensional<N>::Coordinates<T>;

template <Index N>
using Indices = Coordinates<Index, N>;

} // namespace Linx

#endif
