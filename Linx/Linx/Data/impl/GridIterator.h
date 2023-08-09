// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_IMPL_GRIDITERATOR_H
#define _LINXDATA_IMPL_GRIDITERATOR_H

#include "Linx/Data/Grid.h"

namespace Linx {

template <Index N>
class Grid<N>::Iterator : public std::iterator<std::input_iterator_tag, Position<N>> {

public:
  explicit Iterator(const Grid<N>& region, Position<N> current) : m_region(region), m_current(current) {}

  static Iterator begin(const Grid<N>& region) {
    return Iterator(region, region.front());
  }

  static Iterator end(const Grid<N>& region) {
    return Iterator(region, Box<N>::Iterator::end_position(region.box()));
  }

  const Position<N>& operator*() const {
    return m_current;
  }

  const Position<N>* operator->() const {
    return &m_current;
  }

  Iterator& operator++() {
    if (m_current == m_region.back()) {
      m_current = Box<N>::Iterator::end_position(m_region.box());
      return *this;
    }
    m_current[0] += m_region.step()[0];
    for (std::size_t i = 0; i < m_current.size() - 1; ++i) {
      if (m_current[i] > m_region.back()[i]) {
        m_current[i] = m_region.front()[i];
        m_current[i + 1] += m_region.step()[i + 1];
      }
    }
    return *this;
  }

  Iterator operator++(int) {
    auto out = *this;
    ++this;
    return out;
  }

  bool operator==(const Iterator& rhs) const {
    return m_current == rhs.m_current;
  }

  bool operator!=(const Iterator& rhs) const {
    return m_current != rhs.m_current;
  }

private:
  /**
   * @brief The screened region.
   */
  const Grid<N>& m_region;

  /**
   * @brief The current position.
   */
  Position<N> m_current;
};

} // namespace Linx

#endif
