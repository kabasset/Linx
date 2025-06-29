// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_CONCEPTS_ARITHMETIC_H
#define LINX_BASE_CONCEPTS_ARITHMETIC_H

namespace Linx {

/**
 * @brief Concept for additivity, i.e. addable and subtractable types.
 */
template <typename T, typename U>
concept Additive = requires(T lhs, U rhs) {
  ++lhs;
  --lhs;
  lhs++;
  lhs--;
  lhs += rhs;
  lhs -= rhs;
  lhs + rhs;
  lhs - rhs;
};

template <typename T0, typename T1>
concept NotConvertibleTo = not std::convertible_to<T0, T1>;

} // namespace Linx

#endif
