/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_BOX_H
#define _LINXDATA_BOX_H

#include "Linx/Data/Vector.h"

#include <boost/operators.hpp>

namespace Linx {

/// @cond
namespace Internal {
template <Index N>
class BorderedBox; // for friendness // FIXME rm?
}
/// @endcond

/**
 * @ingroup regions
 * @brief An ND bounding box, defined by its front and back positions, both inclusive.
 * 
 * Like `Position`, this class stores no pixel values, but coordinates.
 */
template <Index N = 2>
class Box : boost::additive<Box<N>, Box<N>>, boost::additive<Box<N>, Position<N>>, boost::additive<Box<N>, Index> {
  friend class Internal::BorderedBox<N>; // FIXME rm? make private class, friend of FilterMixin?

public:
  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = N;

  /**
   * @brief A position iterator.
   * 
   * The scanning order maximizes data locality for row-major ordered data like rasters.
   * 
   * That is, the increment operator is such that the corresponding offset in a raster is always increasing.
   * In particular, when screening a whole raster, pixels are visited in the storage order.
   */
  class Iterator;

  /// @{
  /// @group_construction

  Box() : Box(Position<N>::one(), Position<N>::zero()) {}

  /**
   * @brief Constructor.
   */
  Box(Position<N> front, Position<N> back) : m_front(std::move(front)), m_back(std::move(back)) {}

  /**
   * @brief Create a box from a front position and shape.
   */
  static Box<N> from_shape(Position<N> front, Position<N> shape) {
    return {front, front + shape - 1};
  }

  /**
   * @brief Create a box from a radius and center position.
   */
  static Box<N> from_center(Index radius = 1, const Position<N> center = Position<N>::zero()) {
    return {center - radius, center + radius};
  }

  /**
   * @brief Create a conventionally unlimited box.
   * 
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
   * @brief `Position::one()` for compatibility with `Grid`.
   */
  Position<N> step() const {
    return Position<N>::one();
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
    return shape_size(shape());
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

  /// @group_elements

  /**
   * @brief Get the absolute position given a position in the box referential.
   */
  inline Position<N> operator[](const Position<N>& p) const {
    return p + m_front;
  }

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
   * @brief Check whether a position lies inside the box.
   */
  bool contains(const Position<N>& position) const {
    for (Index i = 0; i < dimension(); ++i) {
      if (position[i] < m_front[i] || position[i] > m_back[i]) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Create a list of boxes around the box.
   * @param margin The extent of the surrounding
   * 
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
   * 
   * The back of the box is set to the same coordinate as the front along the axis.
   */
  Box<N>& project(Index axis = 0) {
    m_back[axis] = m_front[axis];
    return *this;
  }

  /**
   * @brief Check whether the box is contained within another box.
   */
  bool operator<=(const Box<N>& rhs) const {
    for (Index i = 0; i < dimension(); ++i) {
      if (m_front[i] < rhs.front()[i]) {
        return false;
      }
      if (m_back[i] > rhs.back()[i]) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Check whether the box is stricly contained within another box.
   */
  bool operator<(const Box<N>& rhs) const {
    for (Index i = 0; i < dimension(); ++i) {
      if (m_front[i] <= rhs.front()[i]) {
        return false;
      }
      if (m_back[i] >= rhs.back()[i]) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Check whether the box contains another box.
   */
  bool operator>=(const Box<N>& rhs) const {
    for (Index i = 0; i < dimension(); ++i) {
      if (m_front[i] > rhs.front()[i]) {
        return false;
      }
      if (m_back[i] < rhs.back()[i]) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Check whether the box stricly contains another box.
   */
  bool operator>(const Box<N>& rhs) const {
    for (Index i = 0; i < dimension(); ++i) {
      if (m_front[i] >= rhs.front()[i]) {
        return false;
      }
      if (m_back[i] <= rhs.back()[i]) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Shrink the box inside another box (i.e. get the intersection of both).
   */
  Box<N>& operator&=(const Box<N>& rhs) {
    for (Index i = 0; i < dimension(); ++i) {
      m_front[i] = std::max(m_front[i], rhs.front()[i]);
      m_back[i] = std::min(m_back[i], rhs.back()[i]);
    }
    return *this;
  }

  /**
   * @brief Minimally grow the box to include another box (i.e. get the minimum box which contains both).
   */
  Box<N>& operator|=(const Box<N>& rhs) {
    for (Index i = 0; i < dimension(); ++i) {
      m_front[i] = std::min(m_front[i], rhs.front()[i]);
      m_back[i] = std::max(m_back[i], rhs.back()[i]);
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
   * @brief Translate the box by a given vector.
   */
  Box<N>& operator+=(const Position<N>& vector) {
    m_front += vector;
    m_back += vector;
    return *this;
  }

  /**
   * @brief Translate the box by the opposite of a given vector.
   */
  Box<N>& operator-=(const Position<N>& vector) {
    m_front -= vector;
    m_back -= vector;
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
 * @relatesalso Box
 * @brief Flatten the box along a given axis.
 */
template <Index N>
Box<N> project(const Box<N>& in, Index axis = 0) {
  auto out = in;
  return out.project(axis);
}

/**
 * @relatesalso Box
 * @brief Erase an axis.
 * @tparam The index of the axis
 */
template <Index I, Index N>
Box<N == -1 ? -1 : N - 1> erase(const Box<N>& in) {
  return {erase<I>(in.front()), erase<I>(in.back())};
}

/**
 * @relatesalso Box
 * @brief Insert an axis.
 * @tparam I The index of the new axis
 * @param front The front bound along axis `I`
 * @param back The back bound along axis `I`
 */
template <Index I, Index N>
Box<N == -1 ? -1 : N + 1> insert(const Box<N>& in, Index front, Index back) {
  return {insert<I>(in.front(), front), insert<I>(in.back(), back)};
}

/**
 * @relatesalso Box
 * @brief Clamp a position inside a box.
 */
template <typename T, Index N = 2>
Vector<T, N> clamp(const Vector<T, N>& position, const Box<N>& box) {
  Vector<T, N> out(box.dimension());
  for (std::size_t i = 0; i < out.dimension(); ++i) {
    out[i] = clamp(position[i], box.front[i], box.back[i]); // TODO transform
  }
  return out;
}

/**
 * @relatesalso Box
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
 * @brief A set of positions which can be translated and clamped (intersected by a box).
 * 
 * TODO
 * - Region += Position
 * - Region -= Position
 * - Region &= Box
 * - box(Region) -> Box
 */

/**
 * @relatesalso Box
 * @brief Identity, for compatibility with `Region`.
 */
template <Index N>
inline const Box<N>& box(const Box<N>& region) {
  return region;
}

/**
 * @relatesalso Box
 * @brief Get the bounding box of a region.
 * 
 * This generic implementation is unoptimized:
 * it iterates over all of the positions.
 */
template <typename TIn>
inline Box<TIn::Dimension> box(const TIn& region) {
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

/**
 * @relatesalso Box
 * @brief Get the intersection of two boxes.
 */
template <Index N>
inline Box<N> operator&(const Box<N>& region, const Box<N>& bounds) {
  auto out = region;
  out &= bounds;
  return out;
}

} // namespace Linx

#include "Linx/Data/impl/BoxIterator.h"

#endif
