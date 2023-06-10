// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_IMPL_BOXITERATOR_H
#define _LINXDATA_IMPL_BOXITERATOR_H

#include "Linx/Data/Box.h"

namespace Linx {

template <Index N>
class Box<N>::Iterator : public std::iterator<std::input_iterator_tag, Position<N>> {

public:
  /**
   * @brief Constructor.
   */
  explicit Iterator(const Box<N>& region, Position<N> current) : m_region(region), m_current(current) {}

  /**
   * @brief The beginning iterator.
   */
  static Iterator begin(const Box<N>& box) {
    return Iterator(box, Iterator::beginPosition(box));
  }

  /**
   * @brief The end iterator.
   */
  static Iterator end(const Box<N>& box) {
    return Iterator(box, Iterator::endPosition(box));
  }

  /**
   * @brief The beginning position.
   */
  static Position<N> beginPosition(const Box<N>& box) {
    return box.front();
  }

  /**
   * @brief The end position.
   */
  static Position<N> endPosition(const Box<N>& box) {
    auto out = box.front();
    out[0] -= (box.size() > 0); // out = front if size <= 0
    return out;
  }

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
   * 
   * Move the current position by 1 pixel,
   * such that the corresponding index in a raster would be increased to the next one.
   */
  Iterator& operator++() {
    if (m_current == m_region.back()) { // TODO simpler?
      m_current = endPosition(m_region);
      return *this;
    }
    ++m_current[0];
    for (std::size_t i = 0; i < m_current.size() - 1; ++i) {
      if (m_current[i] > m_region.back()[i]) {
        m_current[i] = m_region.front()[i];
        ++m_current[i + 1];
      }
    }
    return *this;
  }

  /**
   * @brief Increment operator.
   */
  Iterator operator++(int) {
    auto out = *this;
    ++*this;
    return out;
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
   * @brief The screened region.
   */
  const Box<N>& m_region;

  /**
   * @brief The current position.
   */
  Position<N> m_current;
};

} // namespace Linx

#endif
