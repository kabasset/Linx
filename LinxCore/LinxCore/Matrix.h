// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXCORE_MATRIX_H
#define _LINXCORE_MATRIX_H

#include "Linx/Base/DataContainer.h"
#include "Linx/Base/Exceptions.h"
#include "LinxCore/Vector.h"

#include <eigen3/Eigen/Core>

namespace Linx {

/**
 * @ingroup data_classes
 * @brief Euclidean matrix.
 * 
 * @tparam T The value type
 * @tparam N The number of rows
 * @tparam M The number of columns
 * 
 * 
 * TODO
 */
template <typename T, Index N = 2, Index M = N>
class Matrix :
    public DataContainer<
        T,
        StdHolder<Coordinates<T, N * M>>,
        EuclidArithmetic,
        Matrix<T, N, M>> { // FIXME MatrixArithmetic

public:
  /**
   * @brief The pixel value type.
   */
  using Value = T;

  /**
   * @brief The container type.
   */
  using Container = DataContainer<T, StdHolder<Coordinates<T, N * M>>, EuclidArithmetic, Matrix<T, N, M>>;

  static constexpr Index Rows = N;
  static constexpr Index Columns = M;
  static constexpr Index Rank = std::min(N, M);

  /// @{
  /// @group_construction

  LINX_VIRTUAL_DTOR(Matrix)
  LINX_DEFAULT_COPYABLE(Matrix)
  LINX_DEFAULT_MOVABLE(Matrix)

  /**
   * @brief Constructor.
   */
  explicit Matrix() : Container(N * M), m_eigen(this->data()) {}

  /**
   * @brief Create the identity matrix.
   */
  static Matrix identity() {
    Matrix out;
    auto d = out.data();
    for (Index i = 0; i < Rank; ++i, d += Columns + 1) {
      *d = T(1);
    }
    return out;
  }

  /**
   * @brief Create a diagonal matrix.
   */
  template <typename TRange>
  static Matrix diagonal(const TRange& values) {
    Matrix out;
    auto d = out.data();
    for (auto it = values.begin(); it != values.end(); ++it, d += Columns + 1) {
      *d = *it;
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
  inline const T& operator()(Index row, Index column) const {
    return column + Columns * row;
  }

  /**
   * @copybrief operator()()
   */
  inline T& operator()(Index row, Index column) {
    return const_cast<T&>(const_cast<const Matrix&>(*this)->at(row, column));
  }

  /**
   * @copybrief operator()()
   */
  template <Index R, Index C>
  const T& at() const {
    return C + Columns * R;
  }

  /// @group_operations

  T determinant() const {
    return m_eigen.determinant();
  }

  /// @group_modifier

  Matrix& inverse() {
    m_eigen.inverse();
    return *this;
  }

  /// @}

private:
  Eigen::Map<Eigen::Matrix<T, N, M, Eigen::RowMajor>> m_eigen;
};

} // namespace Linx

#endif
