// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_MASKITERATOR_H
#define _LITLRASTER_MASKITERATOR_H

#include "LitlRaster/Mask.h"

namespace Litl {

template <Index N>
class Mask<N>::Iterator : public std::iterator<std::input_iterator_tag, Position<N>> {

private:
  /**
   * @brief Constructor.
   */
  explicit Iterator(const Mask<N>& region, std::vector<bool>::const_iterator current) :
      m_flagIt(current), m_flagEnd(region.m_flags.end()),
      m_current(region.m_box, Box<N>::Iterator::beginPosition(region.m_box)) {
    while (m_flagIt != m_flagEnd && not *m_flagIt) {
      ++m_flagIt;
      ++m_current;
    }
  }

public:
  /**
   * @brief Make an iterator to the beginning of a mask.
   */
  static Iterator begin(const Mask<N>& region) {
    return Iterator(region, region.m_flags.begin());
  }

  /**
   * @brief Make an iterator to the end of a mask.
   */
  static Iterator end(const Mask<N>& region) {
    return Iterator(region, region.m_flags.end());
  }

  /**
   * @brief Dereference operator.
   */
  const Position<N>& operator*() const {
    return *m_current;
  }

  /**
   * @brief Arrow operator.
   */
  const Position<N>* operator->() const {
    return &*m_current;
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
    return *m_flagIt == *rhs.m_flagIt;
  }

  /**
   * @brief Non-equality operator.
   */
  bool operator!=(const Iterator& rhs) const {
    return *m_flagIt != *rhs.m_flagIt;
  }

private:
  /**
   * @brief Update and get the current position.
   */
  inline const Position<N>& next() {
    do {
      ++m_flagIt;
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
  return Mask<N>::Iterator::begin(region);
}

/**
 * @relates Mask
 * @brief Iterator to one past the back position.
 */
template <Index N>
typename Mask<N>::Iterator end(const Mask<N>& region) {
  return Mask<N>::Iterator::end(region);
}

} // namespace Litl

#endif
