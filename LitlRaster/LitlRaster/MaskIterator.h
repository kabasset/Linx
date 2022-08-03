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
  Iterator& operator++() {
    do {
      ++m_flagIt;
      ++m_current;
    } while (m_flagIt != m_flagEnd && not *m_flagIt);
    return *this;
  }

  /**
   * @brief Increment operator.
   */
  Iterator operator++(int) {
    auto out = *this;
    ++(*this);
    return out;
  }

  /**
   * @brief Equality operator.
   */
  bool operator==(const Iterator& rhs) const {
    return m_flagIt == rhs.m_flagIt;
  }

  /**
   * @brief Non-equality operator.
   */
  bool operator!=(const Iterator& rhs) const {
    return m_flagIt != rhs.m_flagIt;
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

} // namespace Litl

#endif
