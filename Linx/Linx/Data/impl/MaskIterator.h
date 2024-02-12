// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXDATA_IMPL_MASKITERATOR_H
#define _LINXDATA_IMPL_MASKITERATOR_H

#include "Linx/Data/Mask.h"

namespace Linx {

template <Index N>
class Mask<N>::Iterator : public std::iterator<std::input_iterator_tag, Position<N>> {
private:

  /**
   * @brief Constructor.
   */
  explicit Iterator(const Mask<N>& region, const bool* current) :
      m_flag_it(current), m_flag_end(region.m_flags.end()),
      m_current(region.m_box, Box<N>::Iterator::begin_position(region.m_box))
  {
    while (m_flag_it != m_flag_end && not *m_flag_it) {
      ++m_flag_it;
      ++m_current;
    }
  }

public:

  /**
   * @brief Make an iterator to the beginning of a mask.
   */
  static Iterator begin(const Mask<N>& region)
  {
    return Iterator(region, region.m_flags.begin());
  }

  /**
   * @brief Make an iterator to the end of a mask.
   */
  static Iterator end(const Mask<N>& region)
  {
    return Iterator(region, region.m_flags.end());
  }

  /**
   * @brief Dereference operator.
   */
  const Position<N>& operator*() const
  {
    return *m_current;
  }

  /**
   * @brief Arrow operator.
   */
  const Position<N>* operator->() const
  {
    return &*m_current;
  }

  /**
   * @brief Increment operator.
   */
  Iterator& operator++()
  {
    do {
      ++m_flag_it;
      ++m_current;
    } while (m_flag_it != m_flag_end && not *m_flag_it);
    return *this;
  }

  /**
   * @brief Increment operator.
   */
  Iterator operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  /**
   * @brief Equality operator.
   */
  bool operator==(const Iterator& rhs) const
  {
    return m_flag_it == rhs.m_flag_it;
  }

  /**
   * @brief Non-equality operator.
   */
  bool operator!=(const Iterator& rhs) const
  {
    return m_flag_it != rhs.m_flag_it;
  }

private:

  /**
   * @brief The current flag iterator.
   */
  const bool* m_flag_it;
  /**
   * @brief The flag iterator end.
   */
  const bool* m_flag_end;

  /**
   * @brief The current box iterator.
   */
  typename Box<N>::Iterator m_current;
};

} // namespace Linx

#endif
