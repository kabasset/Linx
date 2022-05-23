// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_POSITION_H
#define _RASTER_POSITION_H

#include "Raster/DataContainer.h"
#include "Raster/DataUtils.h"

#include <numeric> // accumulate, multiplies
#include <type_traits> // conditional

namespace Cnes {

/**
 * @brief The signed integer type which represents indices in a raster.
 */
using Index = long;

/**
 * @relates Position
 * @brief The index container type.
 */
template <Index N = 2>
using Indices = typename std::conditional<(N == -1), std::vector<Index>, std::array<Index, (std::size_t)N>>::type;

/**
 * @ingroup data_classes
 * @brief N-dimensional pixel position or image shape, i.e. set of integer coordinates.
 * @tparam N A non-negative dimension (0 is allowed), or -1 for variable dimension.
 * @details
 * The values are stored in a `std::array<Index, N>` in general (`N &ge; 0`),
 * or `std::vector<Index>` for variable dimension (`N = -1`).
 *
 * Memory and services are optimized when dimension is fixed at compile-time (`N &ge; 0`).
 * 
 * Anonymous brace-initialization is permitted, e.g.:
 * \code
 * VecRaster<float> raster({1920, 1080});
 * // Is equivalent to
 * VecRaster<float> raster(Position<2>({1920, 1080}));
 * \endcode
 * 
 * Classical positions are instantiated with named constructors, e.g.:
 * \code
 * auto bottomLeft = Position<2>::zero();
 * auto topRight = Position<2>::max();
 * \endcode
 * 
 * @see Region
 */
template <Index N = 2>
class Position : public DataContainer<Index, DataContainerHolder<Index, Indices<N>>, Position<N>> {
public:
  /**
   * @brief The dimension template parameter.
   */
  static constexpr Index Dim = N;

  CNES_VIRTUAL_DTOR(Position)
  CNES_DEFAULT_COPYABLE(Position)
  CNES_DEFAULT_MOVABLE(Position)

  /**
   * @brief Default constructor.
   * @warning
   * The indices are unspecified.
   * To create position 0, use `zero()` instead.
   */
  Position() : DataContainer<Index, DataContainerHolder<Index, Indices<N>>, Position<N>>() {}

  /**
   * @brief Create a position of given dimension.
   */
  explicit Position(Index dim) : DataContainer<Index, DataContainerHolder<Index, Indices<N>>, Position<N>>(dim) {}

  /**
   * @brief Create a position from a brace-enclosed list of indices.
   */
  Position(std::initializer_list<Index> indices) :
      DataContainer<Index, DataContainerHolder<Index, Indices<N>>, Position<N>>(indices) {}

  /**
   * @brief Create a position from an iterable.
   */
  template <typename TIterable, typename std::enable_if_t<isIterable<TIterable>::value>* = nullptr>
  explicit Position(TIterable&& iterable) :
      DataContainer<Index, DataContainerHolder<Index, Indices<N>>, Position<N>>(iterable) {}

  /**
   * @brief Create position 0.
   */
  static Position<N> zero() {
    Position<N> res(std::abs(Dim));
    std::fill(res.begin(), res.end(), 0);
    return res;
  }

  /**
   * @brief Create a position full of 1's.
   */
  static Position<N> one() {
    Position<N> res(std::abs(Dim));
    std::fill(res.begin(), res.end(), 1);
    return res;
  }

  /**
   * @brief Create max position (full of -1's).
   */
  static Position<N> max() {
    Position<N> res(std::abs(N));
    std::fill(res.begin(), res.end(), -1);
    return res;
  }

  /**
   * @brief Check whether the position is zero.
   */
  bool isZero() const {
    for (auto i : *this) {
      if (i != 0) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Check whether the position is one.
   */
  bool isOne() const {
    for (auto i : *this) {
      if (i != 1) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Check whether the position is max.
   */
  bool isMax() const {
    for (auto i : *this) {
      if (i != -1) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Create a position of lower dimension.
   * @tparam M The new dimension; cannot be -1
   * @details
   * The indices up to dimension `M` are copied.
   */
  template <Index M>
  Position<M> slice() const {
    Position<M> res(M);
    std::copy_n(this->data(), M, res.data());
    return res;
  }

  /**
   * @brief Create a position of higher dimension.
   * @tparam M The new dimension; cannot be -1
   * @details
   * The indices up to dimension `N` are copied.
   * Those between dimensions `N` and `M` are taken from the given position.
   */
  template <Index M>
  Position<M> extend(const Position<M>& padding = Position<M>::zero()) const {
    auto res = padding;
    std::copy_n(this->data(), this->size(), res.data());
    return res;
  }
};

/**
 * @relates Position
 * @brief Compute the number of pixels in a given shape.
 */
template <Index N = 2>
Index shapeSize(const Position<N>& shape) {
  if (shape.size() == 0) {
    return 0;
  }
  return std::accumulate(shape.begin(), shape.end(), 1L, std::multiplies<Index>());
}

} // namespace Cnes

#endif // _RASTER_POSITION_H
