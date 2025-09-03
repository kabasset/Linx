// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_VECTOR_H
#define LINX_DATA_VECTOR_H

#include "Linx/Base/Dimension.h"
#include "Linx/Base/Slice.h"
#include "Linx/Data/Vector/VectorBase.h"

#include <array>
#include <concepts>
#include <utility> // integer_sequence
#include <vector>

namespace Linx {

/**
 * @brief Non-resizable, rank-1 container on host.
 * @tparam T The coefficients specification
 */
template <typename T = std::integer_sequence<int>> // FIXME rm default tparam
class Vector : public VectorBase<T> {
public:

  static constexpr auto n = VectorBase<T>::n; ///< The size parameter
  using data_type = T; ///< The coefficients specification

  using typename VectorBase<T>::size_type;
  using typename VectorBase<T>::ssize_type;
  using typename VectorBase<T>::value_type;
  using typename VectorBase<T>::reference;
  using typename VectorBase<T>::const_reference;
  using typename VectorBase<T>::base_type;

  using memory_space = Kokkos::HostSpace; ///< The memory space
  using execution_space = Kokkos::DefaultHostExecutionSpace; ///< The execution space

  static constexpr bool static_size_flag = (n >= 0); ///< Static size flag
  static constexpr bool static_empty_flag = (n == 0); ///< Statically empty flag
  static constexpr bool static_flag = (n == 0) || std::is_same_v<base_type, void>; ///< Static coefficients flag

  /**
   * @brief Constructor.
   */
  using VectorBase<T>::VectorBase;

  /**
   * @brief Vector domain: `Slice(0, size())`
   */
  constexpr auto domain() const
  {
    return Slice(0, this->size());
  }

  /**
   * @brief Access the i-th coefficient.
   */
  constexpr decltype(auto) operator()(std::integral auto i) const
  {
    return this->operator[](i);
  }

  /**
   * @brief I-th element or a fallback if i is out of bounds.
   */
  template <typename TFallback>
  constexpr TFallback get_or(std::integral auto i, TFallback fallback) const
  {
    if constexpr (std::is_signed_v<decltype(i)>) {
      return i < 0 || i >= this->size() ? fallback : this->operator[](i);
    } else {
      return i >= this->size() ? fallback : this->operator[](i);
    }
  }

  /**
   * @brief Equality comparison with a pack of coefficients.
   */
  constexpr bool equal(auto... coefs) const // TODO equality comparable with value_type
  {
    return this->size() == sizeof...(coefs)
        && equal_impl(forward_as_tuple(coefs...), std::make_index_sequence<sizeof...(coefs)>());
  }

  /**
   * @brief Equality operator.
   */
  constexpr bool operator==(const auto& rhs) const
  {
    // FIXME rm cases 0?

    if (this->size() == 0) {
      for (std::size_t i = 0; i < std::size(rhs); ++i) {
        if (rhs[i] != 0) {
          return false;
        }
      }
      return true;
    }

    if (std::size(rhs) == 0) {
      for (std::size_t i = 0; i < this->size(); ++i) {
        if (this->operator[](i) != 0) {
          return false;
        }
      }
      return true;
    }

    if (this->size() != std::size(rhs)) {
      return false;
    }

    for (std::size_t i = 0; i < this->size(); ++i) {
      if (this->operator[](i) != rhs[i]) {
        return false;
      }
    }

    return true;
  }

private:

  /**
   * @brief Helper method to unroll coefficients.
   */
  template <std::size_t... Is>
  constexpr bool equal_impl([[maybe_unused]] auto coefs, std::index_sequence<Is...>) const
  {
    return ((operator()(Is) == get<Is>(coefs)) && ...);
  }
};

} // namespace Linx

#include "Linx/Data/Vector/arithmetics.h"
#include "Linx/Data/Vector/creation.h"
#include "Linx/Data/Vector/funcs.h"

#endif