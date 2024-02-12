// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXDATA_IMPL_LINEITERATOR_H
#define _LINXDATA_IMPL_LINEITERATOR_H

#include "Linx/Data/Line.h"

namespace Linx {

template <Index I, Index N>
class Line<I, N>::Iterator : public std::iterator<std::input_iterator_tag, Position<N>> {
public:

  explicit Iterator(const Line<I, N>& region, Index current) : m_step(region.step()), m_current(region.front())
  {
    m_current[I] = current;
  }

  static Iterator begin(const Line<I, N>& region)
  {
    return Iterator(region, region.front_index());
  }

  static Iterator end(const Line<I, N>& region)
  {
    return Iterator(region, region.back_index() + region.step());
  }

  const Position<N>& operator*() const
  {
    return m_current;
  }

  const Position<N>* operator->() const
  {
    return &m_current;
  }

  Iterator& operator++()
  {
    m_current[I] += m_step;
    return *this;
  }

  Iterator operator++(int)
  {
    auto out = *this;
    ++this;
    return out;
  }

  bool operator==(const Iterator& rhs) const
  {
    return m_current == rhs.m_current;
  }

  bool operator!=(const Iterator& rhs) const
  {
    return m_current != rhs.m_current;
  }

private:

  /**
   * @brief The step.
   */
  Index m_step;

  /**
   * @brief The current position.
   */
  Position<N> m_current;
};

} // namespace Linx

#endif
