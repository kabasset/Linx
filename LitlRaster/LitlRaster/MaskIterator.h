// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_MASKITERATOR_H
#define _LITLRASTER_MASKITERATOR_H

#include "LitlRaster/Mask.h"

namespace Litl {

template <Index N>
class Mask<N>::Iterator : public std::iterator<std::input_iterator_tag, Position<N>> {

public:
  explicit Iterator(const Mask<N>& region, const Position<N>& current) :
      m_flagIt(region.m_flags.begin()), m_flagEnd(region.m_flags.end()), m_current(region.box(), current) {
    while (m_flagIt != m_flagEnd && not *m_flagIt) {
      ++m_flagIt;
      ++m_current;
    }
  }

  static Position<N> beginPosition(const Mask<N>& region) {
    return Box<N>::Iterator::beginPosition(region.box());
  }

  static Position<N> endPosition(const Mask<N>& region) {
    return Box<N>::Iterator::endPosition(region.box());
  }

  const Position<N>& operator*() const {
    return *m_current;
  }

  const Position<N>* operator->() const {
    return &*m_current;
  }

  const Position<N>& operator++() {
    return next();
  }

  const Position<N>* operator++(int) {
    return &next();
  }

  bool operator==(const Iterator& rhs) const {
    return *m_current == *rhs.m_current;
  }

  bool operator!=(const Iterator& rhs) const {
    return *m_current != *rhs.m_current;
  }

private:
  inline const Position<N>& next() {
    do {
      ++m_current;
    } while (m_flagIt != m_flagEnd && not *m_flagIt);
    return *m_current;
  }

private:
  /**
   * @brief The current flag iterator.
   */
  std::vector<bool>::const_iterator m_flagIt;
  /**
   * @brief The flag iterator end.
   */
  std::vector<bool>::const_iterator m_flagEnd;

  /**
   * @brief The current box iterator.
   */
  typename Box<N>::Iterator m_current;
};

/**
 * @relates Mask
 * @brief Iterator to the front position.
 */
template <Index N>
typename Mask<N>::Iterator begin(const Mask<N>& region) {
  return typename Mask<N>::Iterator(region, Mask<N>::Iterator::beginPosition(region));
}

/**
 * @relates Mask
 * @brief Iterator to one past the back position.
 */
template <Index N>
typename Mask<N>::Iterator end(const Mask<N>& region) {
  return typename Mask<N>::Iterator(region, Mask<N>::Iterator::endPosition(region));
}

} // namespace Litl

#endif
