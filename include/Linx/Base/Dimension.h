// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_DIMENSION_H
#define LINX_BASE_DIMENSION_H

namespace Linx {

/**
 * @brief Strong type for a dimension.
 */
struct Dimension {
  int value; ///< The value
};

/**
 * @brief User-defined literals.
 */
inline namespace Literals {

/**
 * @brief Parse an `int` literal given as a static list of characters.
 */
template <char... Cs>
  requires((Cs >= '0' && Cs <= '9') && ...)
constexpr int parse_int_literal()
{
  int out = 0;
  return ((out = out * 10 + (Cs - '0')), ...);
}

/**
 * @brief User-defined literal for a `Dimension`.
 */
template <char... Cs>
constexpr auto operator""_D()
{
  return Dimension {parse_int_literal<Cs...>()};
}

} // namespace Literals
} // namespace Linx

#endif