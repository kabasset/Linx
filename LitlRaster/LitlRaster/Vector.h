// Copyright (C) 2022, Antoine Basset
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_VECTOR_H
#define _LITLRASTER_VECTOR_H

#include "LitlContainer/DataContainer.h"
#include "LitlTypes/TypeUtils.h"

#include <numeric> // accumulate, multiplies
#include <type_traits> // conditional

namespace Litl {

/**
 * @brief A container of coordinates.
 */
template <typename T, Index N = 2>
using Coordinates = typename std::conditional<(N == -1), std::vector<T>, std::array<T, (std::size_t)N>>::type;

/**
 * @relates Position
 * @brief The index container type.
 */
template <Index N = 2>
using Indices = Coordinates<Index, N>;

/**
 * @ingroup data_classes
 * @brief N-dimensional vector, mainly intended for pixel position or image shape, i.e. set of coordinates.
 * @tparam N A non-negative dimension (0 is allowed), or -1 for variable dimension.
 * @details
 * The values are stored in a `std::array<T, N>` in general (`N &ge; 0`),
 * or `std::vector<T>` for variable dimension (`N = -1`).
 *
 * Memory and services are optimized when dimension is fixed at compile-time (`N &ge; 0`).
 * 
 * @tspecialization{Position}
 */
template <typename T, Index N = 2>
class Vector : public DataContainer<T, StdHolder<Coordinates<T, N>>, VectorArithmetic, Vector<T, N>> {
public:
  /**
   * @brief The value type.
   */
  using Value = T;

  /**
   * @brief The dimension template parameter.
   */
  static constexpr Index Dimension = N;

  /**
   * @brief The container type.
   */
  using Container = DataContainer<T, StdHolder<Coordinates<T, N>>, VectorArithmetic, Vector<T, N>>;

  LITL_VIRTUAL_DTOR(Vector)
  LITL_DEFAULT_COPYABLE(Vector)
  LITL_DEFAULT_MOVABLE(Vector)

  /**
   * @brief Default constructor.
   * @warning
   * The values are unspecified.
   * To create vector 0, use `zero()` instead.
   */
  Vector() : Container() {}

  /**
   * @brief Create a vector of given dimension.
   */
  explicit Vector(T dim) : Container(dim) {}

  /**
   * @brief Create a vector from a brace-enclosed list of indices.
   */
  Vector(std::initializer_list<T> indices) : Container(indices) {}

  /**
   * @brief Create a vector from an iterable.
   */
  template <typename TIterable, typename std::enable_if_t<isIterable<TIterable>::value>* = nullptr>
  explicit Vector(TIterable&& iterable) : Container(iterable) {}

  /**
   * @brief Create a vector full of `Limits::zero()'s.
   */
  static Vector<T, N> zero() {
    Vector<T, N> res(std::abs(N));
    return res.fill(Limits<T>::zero());
  }

  /**
   * @brief Create a vector full of `Limits::one()'s.
   */
  static Vector<T, N> one() {
    Vector<T, N> res(std::abs(N));
    return res.fill(Limits<T>::one());
  }

  /**
   * @brief Create a vector full of `Limits::inf()'s.
   */
  static Vector<T, N> inf() {
    Vector<T, N> res(std::abs(N));
    return res.fill(-Limits<T>::inf());
  }

  /**
   * @brief Check whether the vector is zero.
   */
  bool isZero() const {
    for (auto i : *this) {
      if (i != Limits<T>::zero()) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Check whether the vector is one.
   */
  bool isOne() const {
    for (auto i : *this) {
      if (i != Limits<T>::one()) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Check whether the vector is minus one.
   */
  bool isInf() const {
    for (auto i : *this) {
      if (i != Limits<T>::inf()) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Create a vector of lower dimension.
   * @tparam M The new dimension; cannot be -1
   * @details
   * The values up to dimension `M` are copied.
   */
  template <Index M>
  Vector<T, M> slice() const {
    Vector<T, M> res(M);
    std::copy_n(this->data(), M, res.data());
    return res;
  }

  /**
   * @brief Create a vector of higher dimension.
   * @tparam M The new dimension; cannot be -1
   * @details
   * The values up to dimension `N` are copied.
   * Those between dimensions `N` and `M` are taken from the given padding vector.
   */
  template <Index M>
  Vector<T, M> extend(const Vector<T, M>& padding = Vector<T, M>::zero()) const {
    auto res = padding;
    std::copy_n(this->data(), this->size(), res.data());
    return res;
  }
};

/**
 * @ingroup data_classes
 * @brief Pixel position or image shape, i.e. set of indices.
 * @tparam N A non-negative dimension (0 is allowed), or -1 for variable dimension.
 * @details
 * Anonymous brace-initialization is permitted, e.g.:
 * \code
 * Raster<float> raster({1920, 1080});
 * // Is equivalent to
 * Raster<float> raster(Position<2>({1920, 1080}));
 * \endcode
 * 
 * Classical positions are instantiated with named constructors, e.g.:
 * \code
 * auto bottomLeft = Position<2>::zero();
 * auto topRight = Position<2>::ùinusOne();
 * \endcode
 * 
 * @see Region
 */
template <Index N = 2>
using Position = Vector<Index, N>;

/**
 * @relates Position
 * @brief Get the stride along a given axis.
 */
template <Index N>
Index shapeStride(const Position<N>& shape, Index axis) {
  return std::accumulate(shape.begin(), shape.begin() + axis, 1L, std::multiplies<Index> {});
}

/**
 * @relates Position
 * @brief Get the stride along a given axis.
 */
template <Index Axis, Index N>
Index shapeStride(const Position<N>& shape) {
  return shapeStride(shape, Axis);
}

/**
 * @relates Position
 * @brief Compute the number of pixels in a given shape.
 */
template <Index N = 2>
Index shapeSize(const Position<N>& shape) {
  const auto size = shape.size();
  if (size == 0) {
    return 0;
  }
  return shapeStride(shape, size);
}

} // namespace Litl

#endif
