/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLCORE_ORIENTEDSLICE_H
#define _LITLCORE_ORIENTEDSLICE_H

#include "LitlCore/Box.h"
#include "LitlCore/Vector.h"

#include <boost/operators.hpp>

namespace Litl {

/**
 * @ingroup regions
 * @brief An axis-aligned slice.
 */
template <Index I, Index N = 2>
class OrientedSlice : boost::additive<OrientedSlice<I, N>, Position<N>>, boost::additive<OrientedSlice<I, N>, Index> {

public:
  /**
   * @brief The index of the axis the slice is aligned to.
   */
  static constexpr Index Axis = I;

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
  OrientedSlice(Position<N> front, Index back, Index step = 1) :
      m_front(std::move(front)), m_step(step), m_size((back - m_front[I]) / m_step + 1) {}

  /**
   * @brief Create a slice from a front position, size, and optional step.
   */
  static OrientedSlice<I, N> fromSize(Position<N> front, std::size_t size, Index step = 1) {
    const auto back = front[I] + step * (size - 1);
    return OrientedSlice(std::move(front), back, step);
  }

  /// @group_properties

  /**
   * @brief Compute the bounding box.
   */
  Box<N> box() const {
    return {m_front, back()};
  }

  /**
   * @brief Get the front position.
   */
  const Position<N>& front() const {
    return m_front;
  }

  /**
   * @brief Compute the back position.
   */
  Position<N> back() const {
    auto out = m_front;
    out[I] += m_step * (m_size - 1);
    return out;
  }

  /**
   * @brief Get the front index along the axis.
   */
  Index frontIndex() const {
    return m_front[I];
  }

  /**
   * @brief Compute the back index along the axis.
   */
  Index backIndex() const {
    return m_front[I] + m_step * (m_size - 1);
  }

  /**
   * @brief Get the step.
   */
  Index step() const {
    return m_step;
  }

  /**
   * @brief Get the number of dimensions.
   */
  Index dimension() const {
    return m_front.dimension();
  }

  /**
   * @brief Get the number of positions.
   */
  Index size() const {
    return m_size;
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
   * @brief Check whether two slices are equal.
   */
  bool operator==(const OrientedSlice<I, N>& other) const {
    return m_front == other.m_front && m_step == other.m_step && m_size == other.m_size;
  }

  /**
   * @brief Check whether two slices are different.
   */
  bool operator!=(const OrientedSlice<I, N>& other) const {
    return m_front != other.m_front || m_step != other.m_step || m_size != other.m_size;
  }

  /// @group_modifiers

  /**
   * @brief Translate the box by a given vector.
   */
  OrientedSlice<I, N>& operator+=(const Position<N>& vector) {
    m_front += vector;
    return *this;
  }

  /**
   * @brief Translate the box by the opposite of a given vector.
   */
  OrientedSlice<I, N>& operator-=(const Position<N>& vector) {
    m_front -= vector;
    return *this;
  }

  /**
    * @brief Add a scalar to each coordinate.
    */
  OrientedSlice<I, N>& operator+=(Index scalar) {
    m_front += scalar;
    return *this;
  }

  /**
   * @brief Subtract a scalar to each coordinate.
   */
  OrientedSlice<I, N>& operator-=(Index scalar) {
    m_front -= scalar;
    return *this;
  }

  /**
   * @brief Add 1 to each coordinate.
   */
  OrientedSlice<I, N>& operator++() {
    return *this += 1;
  }

  /**
   * @brief Subtract 1 to each coordinate.
   */
  OrientedSlice<I, N>& operator--() {
    return *this -= 1;
  }

  /**
   * @brief Copy.
   */
  OrientedSlice<I, N> operator+() {
    return *this;
  }

  /**
   * @brief Invert the sign of each coordinate.
   */
  OrientedSlice<I, N> operator-() {
    return OrientedSlice<I, N>::fromSize(-m_front, m_size, -m_step);
  }

  /// @}

private:
  /**
   * @brief The front position.
   */
  Position<N> m_front;

  /**
   * @brief The step.
   */
  Index m_step;

  /**
   * @brief The number of positions.
   */
  std::size_t m_size;
};

/**
 * @relates OrientedSlice
 * @brief Get the bounding box of a slice.
 */
template <Index I, Index N>
inline const Box<N>& box(const OrientedSlice<I, N>& region) {
  return region.box();
}

} // namespace Litl

#include "LitlCore/impl/OrientedSliceIterator.h"

#endif
