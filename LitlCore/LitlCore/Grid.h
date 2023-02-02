/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLCORE_GRID_H
#define _LITLCORE_GRID_H

#include "LitlCore/Box.h"

#include <boost/operators.hpp>

namespace Litl {

/**
 * @ingroup regions
 * @brief An ND regular grid.
 */
template <Index N = 2>
class Grid : boost::additive<Grid<N>, Position<N>>, boost::additive<Grid<N>, Index> {

public:
  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = N;

  /**
   * @brief A position iterator.
   * @see `Box::Iterator`
   */
  class Iterator;

  /// @{
  /// @group_construction

  /**
   * @brief Constructor.
   */
  Grid(Box<N> box, Position<N> step) : m_box(std::move(box)), m_step(std::move(step)) {
    Position<N> back(m_box.back());
    for (std::size_t i = 0; i < back.size(); ++i) {
      back[i] -= (m_box.length(i) - 1) % m_step[i];
    }
    m_box = Box<N>(m_box.front(), back);
  }

  /**
   * @brief Regular grid constructor.
   */
  Grid(Box<N> box, Index step) : Grid(std::move(box), Position<N>(m_box.dimension()).fill(step)) {}

  /**
   * @brief Create a grid from a front position, shape and step.
   * @param front The front position
   * @param shape The number of grid nodes along each axis
   * @param step The steps along each axis
   */
  static Grid<N> fromShape(Position<N> front, Position<N> shape, Position<N> step) {
    auto span = shape;
    for (std::size_t i = 0; i < front.size(); ++i) {
      span[i] *= step[i];
    }
    return Grid<N>(Box<N>::fromShape(front, span), step);
  }

  /// @group_properties

  /**
   * @brief Get the bounding box.
   */
  const Box<N>& box() const {
    return m_box;
  }

  /**
   * @brief Get the front position.
   */
  const Position<N>& front() const {
    return m_box.front();
  }

  /**
   * @brief Get the back position.
   */
  const Position<N>& back() const {
    return m_box.back();
  }

  /**
   * @brief Get the step.
   */
  const Position<N>& step() const {
    return m_step;
  }

  /**
   * @brief Get the number of dimensions.
   */
  Index dimension() const {
    return m_box.dimension();
  }

  /**
   * @brief Get the number of grid nodes along each axis
   */
  Position<Dimension> shape() const {
    Position<Dimension> out(m_box.shape());
    for (std::size_t i = 0; i < out.size(); ++i) {
      out[i] = out[i] / m_step[i] + 1;
    }
    return out;
  }

  /**
   * @brief Get the number of grid nodes.
   */
  Index size() const {
    Index out = length<0>();
    for (Index i = 1; i < dimension(); ++i) {
      out *= length(i);
    }
    return out;
  }

  /**
   * @brief Get the number of nodes along given axis.
   */
  template <Index I>
  Index length() const {
    return (m_box.template length<I>() - 1) / m_step[I] + 1;
  }

  /**
   * @brief Get the number of nodes along given axis.
   */
  Index length(Index i) const {
    return (m_box.length(i) - 1) / m_step[i] + 1;
  }

  /// @group_elements

  /**
   * @brief Get an iterator to the beginning.
   */
  Iterator begin() const {
    return Iterator::begin(*this);
  }

  /**
   * @brief Get an iterator to the end.
   */
  Iterator end() const {
    return Iterator::end(*this);
  }

  /// @group_operations

  /**
   * @brief Check whether two boxes are equal.
   */
  bool operator==(const Grid<N>& other) const {
    return m_box == other.m_box && m_step == other.m_step;
  }

  /**
   * @brief Check whether two boxes are different.
   */
  bool operator!=(const Grid<N>& other) const {
    return m_box != other.m_box || m_step != other.m_step;
  }

  /**
   * @brief Check whether a position is a grid node.
   */
  bool operator[](const Position<N>& position) const {
    for (Index i = 0; i < dimension(); ++i) {
      if (position[i] < m_box.front()[i] || position[i] > m_box.back()[i]) {
        return false;
      }
      if ((position[i] - m_box.front()[i]) % m_step[i] != 0) {
        return false;
      }
    }
    return true;
  }

  /// @group_modifiers

  /**
   * @brief Flatten the grid along a given axis.
   * 
   * The back of the box is set to the same coordinate as the front along the axis.
   */
  Grid<N>& project(Index axis = 0) {
    m_box.project(axis);
    return *this;
  }

  /**
   * @brief Clamp the front and back positions inside a given box.
   */
  Grid<N>& operator&=(const Box<N>& box) {
    *this = *this & box;
    return *this;
  }

  /**
   * @brief Translate the box by a given vector.
   */
  Grid<N>& operator+=(const Position<N>& vector) {
    m_box += vector;
    return *this;
  }

  /**
   * @brief Translate the box by the opposite of a given vector.
   */
  Grid<N>& operator-=(const Position<N>& vector) {
    m_box -= vector;
    return *this;
  }

  /**
    * @brief Add a scalar to each coordinate.
    */
  Grid<N>& operator+=(Index scalar) {
    m_box += scalar;
    return *this;
  }

  /**
   * @brief Subtract a scalar to each coordinate.
   */
  Grid<N>& operator-=(Index scalar) {
    m_box -= scalar;
    return *this;
  }

  /**
   * @brief Add 1 to each coordinate.
   */
  Grid<N>& operator++() {
    return *this += 1;
  }

  /**
   * @brief Subtract 1 to each coordinate.
   */
  Grid<N>& operator--() {
    return *this -= 1;
  }

  /**
   * @brief Copy.
   */
  Grid<N> operator+() {
    return *this;
  }

  /**
   * @brief Invert the sign of each coordinate.
   */
  Grid<N> operator-() {
    return {-m_box, -m_step};
  }

  /// @}

private:
  /**
   * @brief The bounding box.
   */
  Box<N> m_box;

  /**
   * @brief The step along each axis.
   */
  Position<N> m_step;
};

/**
 * @relates Grid
 * @brief Get the bounding box of a grid.
 */
template <Index N>
inline const Box<N>& box(const Grid<N>& region) {
  return region.box();
}

/**
 * @relates Grid
 * @brief Flatten the grid along a given axis.
 */
template <Index N>
Grid<N> project(const Grid<N>& in, Index axis = 0) {
  auto out = in;
  return out.project(axis);
}

/**
 * @relates Grid
 * @brief Clamp a grid inside a bounding box.
 */
template <Index N>
inline Grid<N> operator&(const Grid<N>& region, const Box<N>& bounds) {
  auto front = bounds.front();
  for (Index i = 0; i < region.size(); ++i) {
    front[i] += (region.back()[i] - front[i]) % region.step()[i];
  }
  return Grid<N>({front, bounds.back()}, region.step());
}

} // namespace Litl

#include "LitlCore/impl/GridIterator.h"

#endif
