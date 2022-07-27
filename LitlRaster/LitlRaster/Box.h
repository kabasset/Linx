/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_BOX_H
#define _LITLRASTER_BOX_H

#include "LitlRaster/Vector.h"

#include <boost/operators.hpp>

namespace Litl {

/**
 * @brief An ND bounding box, defined by its front and back positions, both inclusive.
 * @details
 * Like `Position`, this class stores no pixel values, but coordinates.
 */
template <Index N = 2>
class Box : boost::additive<Box<N>, Box<N>>, boost::additive<Box<N>, Position<N>>, boost::additive<Box<N>, Index> {

public:
  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = N;

  /**
   * @brief A position iterator.
   * @details
   * The scanning order maximizes data locality for row-major ordered data like rasters.
   * 
   * That is, the increment operator is such that the corresponding offset in a raster is always increasing.
   * In particular, when screening a whole raster, pixels are visited in the storage order.
   */
  class Iterator;

  /// @{
  /// @group_construction

  /**
   * @brief Constructor.
   */
  Box(Position<N> front, Position<N> back) : m_front(std::move(front)), m_back(std::move(back)) {}

  /**
   * @brief Create a box from a front position and shape.
   */
  static Box<N> fromShape(Position<N> front, Position<N> shape) {
    return {front, front + shape - 1};
  }

  /**
   * @brief Create a box from a radius and center position.
   */
  static Box<N> fromCenter(Index radius = 1, const Position<N> center = Position<N>::zero()) {
    return {center - radius, center + radius};
  }

  /**
   * @brief Create a conventionally unlimited box.
   * @details
   * Front and back bounds along each axis are respectively 0 and inf.
   */
  static Box<N> whole() {
    return {Position<N>::zero(), Position<N>::inf()};
  }

  /// @group_properties

  /**
   * @brief Get the front position.
   */
  const Position<N>& front() const {
    return m_front;
  }

  /**
   * @brief Get the back position.
   */
  const Position<N>& back() const {
    return m_back;
  }

  /**
   * @brief Compute the box shape.
   */
  Position<N> shape() const {
    return m_back - m_front + 1;
  }

  /**
   * @brief Get the number of dimensions.
   */
  Index dimension() const {
    return m_front.size();
  }

  /**
   * @brief Compute the box size, i.e. number of positions.
   */
  Index size() const {
    return shapeSize(shape());
  }

  /**
   * @brief Get the box length along given axis.
   */
  template <Index I>
  Index length() const {
    return m_back[I] - m_front[I] + 1;
  }

  /**
   * @brief Get the box length along given axis.
   */
  Index length(Index i) const {
    return m_back[i] - m_front[i] + 1;
  }

  /// @group_operations

  /**
   * @brief Check whether two boxes are equal.
   */
  bool operator==(const Box<N>& other) const {
    return m_front == other.m_front && m_back == other.m_back;
  }

  /**
   * @brief Check whether two boxes are different.
   */
  bool operator!=(const Box<N>& other) const {
    return m_front != other.m_front || m_back != other.m_back;
  }

  /**
   * @brief Create a list of boxes around the box.
   * @param margin The extent of the surrounding
   * @details
   * The indices of `margin.front` must be negative or null
   * while those of `margin.back` must be positive or null.
   * No empty boxes are created, such that the number of output boxes
   * is less than `2 * in.dimension()` if some indices are null.
   * 
   * The union of all output boxes and the input box is a box such that:
   * `union.front = in.front + margin.front` and `union.back = in.back + margin.back`.
   * Partitioning is optimized for data locality when scanning raster pixels in the boxes.
   */
  std::vector<Box<N>> surround(const Box<N>& margin) const {

    Box<N> current = *this;
    const auto dim = dimension();
    std::vector<Box<N>> out;
    out.reserve(dim * 2);

    for (Index i = 0; i < dim; ++i) {

      // Front
      const auto f = margin.m_front[i];
      if (f < 0) {
        auto before = current;
        before.m_back[i] = current.m_front[i] - 1;
        before.m_front[i] = current.m_front[i] += f;
        out.push_back(before);
      }

      // Back
      const auto b = margin.m_back[i];
      if (b > 0) {
        auto after = current;
        after.m_front[i] = current.m_back[i] + 1;
        after.m_back[i] = current.m_back[i] += b;
        out.push_back(after);
      }
    }

    return out;
  }

  /// @group_modifiers

  /**
   * @brief Flatten the box along a given axis.
   * @details
   * The back of the box is set to the same coordinate as the front along the axis.
   */
  Box<N>& project(Index axis = 0) {
    m_back[axis] = m_front[axis];
    return *this;
  }

  /**
   * @brief Clamp the front and back positions inside a given box.
   */
  Box<N>& clamp(const Box<N>& box) {
    for (Index i = 0; i < size(); ++i) {
      m_front[i] = std::max(m_front[i], box.front()[i]);
      m_back[i] = std::min(m_back[i], box.back()[i]);
    }
    return *this;
  }

  /**
   * @brief Grow the box by a given margin.
   */
  Box<N>& operator+=(const Box<N>& margin) {
    m_front += margin.m_front;
    m_back += margin.m_back;
    return *this;
  }

  /**
   * @brief Shrink the box by a given margin.
   */
  Box<N>& operator-=(const Box<N>& margin) {
    m_front -= margin.m_front;
    m_back -= margin.m_back;
    return *this;
  }

  /**
   * @brief Shift the box by a given vector.
   */
  Box<N>& operator+=(const Position<N>& shift) {
    m_front += shift;
    m_back += shift;
    return *this;
  }

  /**
   * @brief Shift the box by a given vector.
   */
  Box<N>& operator-=(const Position<N>& shift) {
    m_front -= shift;
    m_back -= shift;
    return *this;
  }

  /**
    * @brief Add a scalar to each coordinate.
    */
  Box<N>& operator+=(Index scalar) {
    m_front += scalar;
    m_back += scalar;
    return *this;
  }

  /**
   * @brief Subtract a scalar to each coordinate.
   */
  Box<N>& operator-=(Index scalar) {
    m_front -= scalar;
    m_back -= scalar;
    return *this;
  }

  /**
   * @brief Add 1 to each coordinate.
   */
  Box<N>& operator++() {
    return *this += 1;
  }

  /**
   * @brief Subtract 1 to each coordinate.
   */
  Box<N>& operator--() {
    return *this -= 1;
  }

  /**
   * @brief Copy.
   */
  Box<N> operator+() {
    return *this;
  }

  /**
   * @brief Invert the sign of each coordinate.
   */
  Box<N> operator-() {
    return {-m_front, -m_back};
  }

  /// @}

private:
  /**
   * @brief The front position in the box.
   */
  Position<N> m_front;

  /**
   * @brief The back position in the box.
   */
  Position<N> m_back;
};

/**
 * @relates Box
 * @brief Flatten the box along a given axis.
 */
template <Index N>
Box<N> project(const Box<N>& in, Index axis = 0) {
  auto out = in;
  return out.project(axis);
}

/**
 * @relates Box
 * @brief Clamp a position inside a box.
 */
template <typename T, Index N = 2>
Vector<T, N> clamp(const Vector<T, N>& position, const Box<N>& box) {
  Vector<T, N> out(box.size());
  for (std::size_t i = 0; i < out.size(); ++i) {
    out[i] = clamp(position[i], box.front[i], box.back[i]); // TODO transform
  }
  return out;
}

/**
 * @relates Box
 * @brief Clamp a position inside a shape.
 */
template <typename T, Index N = 2>
Vector<T, N> clamp(const Vector<T, N>& position, const Position<N>& shape) {
  Vector<T, N> out(shape.size());
  std::transform(position.begin(), position.end(), shape.begin(), out.begin(), [](auto p, auto s) {
    return clamp(p, Index(0), s - 1);
  });
  return out;
}

/**
 * @ingroup concepts
 * @requirements{Region}
 * @brief FIXME
 */

/**
 * @relates Box
 * @brief Identity, for compatibility with `Region`.
 */
template <Index N>
inline const Box<N>& box(const Box<N>& region) {
  return region;
}

/**
 * @relates Box
 * @brief Get the bounding box of a region.
 * @details
 * This generic implementation is unoptimized:
 * it iterates over all of the positions.
 */
template <typename TIn>
inline const Box<TIn::Dimension>& box(const TIn& region) {
  Position<TIn::Dimension> front;
  Position<TIn::Dimension> back;
  for (const auto& p : region) {
    for (Index i = 0; i < TIn::Dimension; ++i) {
      front[i] = std::min(front[i], p[i]);
      back[i] = std::max(back[i], p[i]);
    }
  }
  return {front, back};
}

} // namespace Litl

#include "LitlRaster/BoxIterator.h"

#endif
