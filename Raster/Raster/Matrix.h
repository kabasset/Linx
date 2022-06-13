// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_MATRIX_H
#define _RASTER_MATRIX_H

#include "Raster/Arithmetic.h"
#include "Raster/DataContainer.h"
#include "Raster/Position.h" // FIME Vector.h
#include "RasterTypes/Exceptions.h"

namespace Cnes {

/**
 * @ingroup data_classes
 * @brief Euclidean matrix.
 * 
 * @tparam T The value type
 * @tparam N The number of rows
 * @tparam M The number of columns
 * 
 * @details
 * TODO
 */
template <typename T, Index N = 2, Index M = N>
class Matrix :
    public DataContainer<T, Coordinates<T, N * M>, Matrix<T, N, M>>,
    public ArithmeticMixin<EuclidArithmetic, T, Matrix<T, N, M>> { // FIXME MatrixArithmetic

public:
  /**
   * @brief The pixel value type.
   */
  using Value = T;

  static constexpr Index Rows = N;
  static constexpr Index Columns = M;
  static constexpr Index Rank = std::min(N, M);

  /// @{
  /// @group_construction

  CNES_VIRTUAL_DTOR(Matrix)
  CNES_DEFAULT_COPYABLE(Matrix)
  CNES_DEFAULT_MOVABLE(Matrix)

  /**
   * @brief Constructor.
   */
  explicit Matrix() : DataContainer<T, Coordinates<T, N * M>, Matrix<T, N, M>>(N * M) {}

  /**
   * @brief Create the identity matrix.
   */
  static Matrix identity() {
    Matrix out;
    for (Index i = 0; i < Rank; ++i) {
      out(i, i) = 1;
    }
    return out;
  }

  /**
   * @brief Create a diagonal matrix.
   */
  template <typename TIterable>
  static Matrix diagonal(const TIterable& values) {
    Matrix out;
    Index i = 0;
    for (auto e : values) {
      out(i, i) = e;
      ++i;
    }
    return out;
  }

  /// @group_properties

  /**
   * @brief Get the matrix shape.
   */
  const Position<2>& shape() const {
    return {N, M};
  }

  /// @group_elements

  /**
   * @brief Access element at given row and column.
   */
  inline const T& at(Index row, Index column) const {
    return row + column * Rows;
  }

  /**
   * @copybrief at()
   */
  inline T& at(Index row, Index column) {
    return const_cast<T&>(const_cast<const Matrix&>(*this)->at(row, column));
  }

  /// @group_modifier

  Matrix& inverse() {
    // FIXME
    return *this;
  }

  /// @}
};

} // namespace Cnes

#endif
