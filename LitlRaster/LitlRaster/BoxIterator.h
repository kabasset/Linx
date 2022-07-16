// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_BOXITERATOR_H
#define _LITLRASTER_BOXITERATOR_H

#include "LitlRaster/Box.h"

namespace Litl {

template <Index N>
class Box<N>::Iterator : public std::iterator<std::input_iterator_tag, Position<N>> {

public:
  /**
   * @brief Constructor.
   */
  explicit Iterator(const Box<N>& region) : m_region(region), m_current(region.front()) {}

  /**
   * @brief Dereference operator.
   */
  const Position<N>& operator*() const {
    return m_current;
  }

  /**
   * @brief Arrow operator.
   */
  const Position<N>* operator->() const {
    return &m_current;
  }

  /**
   * @brief Increment operator.
   */
  const Position<N>& operator++() {
    return next();
  }

  /**
   * @brief Increment operator.
   */
  const Position<N>* operator++(int) {
    return &next();
  }

  /**
   * @brief Equality operator.
   */
  bool operator==(const Iterator& rhs) const {
    return m_current == rhs.m_current;
  }

  /**
   * @brief Non-equality operator.
   */
  bool operator!=(const Iterator& rhs) const {
    return m_current != rhs.m_current;
  }

  /**
   * @brief Reset the current and followers positions to the initial positions.
   */
  void reset() {
    m_current = m_region.front();
  }

private:
  /**
   * @brief Update and get the current position and that of a follower.
   * @details
   * Move the current position by 1 pixel,
   * such that the corresponding index in a raster would be increased to the next one.
   */
  inline const Position<N>& next() {
    if (m_current == m_region.back()) { // TODO simpler?
      m_current = m_region.front();
      --m_current[0];
      return m_current;
    }
    ++m_current[0];
    for (std::size_t i = 0; i < m_current.size() - 1; ++i) {
      if (m_current[i] > m_region.back()[i]) {
        m_current[i] = m_region.front()[i];
        ++m_current[i + 1];
      }
    }
    return m_current;
  }

private:
  /**
   * @brief The screened region.
   */
  const Box<N>& m_region;

  /**
   * @brief The current position.
   */
  Position<N> m_current;
};

/**
 * @relates Box
 * @brief Iterator to the front position.
 */
template <Index N>
typename Box<N>::Iterator begin(const Box<N>& box) {
  return typename Box<N>::Iterator(box);
}

/**
 * @relates Box
 * @brief Iterator to one past the back position.
 */
template <Index N>
typename Box<N>::Iterator end(const Box<N>& box) {
  auto front = box.front();
  --front[0];
  return typename Box<N>::Iterator({front, box.back()});
}

} // namespace Litl

#endif
