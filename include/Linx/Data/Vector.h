// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_VECTOR_H
#define LINX_DATA_VECTOR_H

#include "Linx/Base/Rank.h"
#include "Linx/Data/Vector/VectorBase.h"

#include <array>
#include <concepts>
#include <iostream>
#include <utility> // integer_sequence
#include <vector>

namespace Linx {

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

} // namespace Linx

#include "Linx/Data/Vector/creation.h"
#include "Linx/Data/Vector/funcs.h"

#endif