// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_SUBRASTERITERATOR_H
#define _LITLRASTER_SUBRASTERITERATOR_H

#include "LitlRaster/Subraster.h"

namespace Litl {

template <typename TRaster, typename T>
class Subraster<TRaster, T>::Iterator : public std::iterator<std::input_iterator_tag, T> {

public:
  using Parent = TRaster;
  using Value = T;
  static constexpr Index Dimension = TRaster::Dimension;

  /**++
   * @brief Constructor.
   */
  Iterator(const Subraster<TRaster, Value>& subraster, const Position<Dimension>& position) :
      m_raster(subraster.raster()), m_beginPlane(project(subraster.region())), m_beginIt(m_beginPlane),
      m_width(subraster.region().length(0)), m_current(&m_raster[position]), m_eol(m_current + m_width) {}

  /**
   * @brief Dereference operator.
   */
  Value& operator*() const {
    return *m_current;
  }

  /**
   * @brief Arrow operator.
   */
  Value* operator->() const {
    return m_current;
  }

  /**
   * @brief Increment operator.
   */
  Value& operator++() {
    return *next();
  }

  /**
   * @brief Increment operator.
   */
  Value* operator++(int) {
    return next();
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

private:
  inline Value* next() {
    if (++m_current != m_eol) {
      return m_current;
    }
    ++m_beginIt;
    // Use data() and index()  instead of [] to avoid dereferencing unallocated memory
    m_current = m_raster.data() + m_raster.index(*m_beginIt);
    m_eol = m_current + m_width;
    return m_current;
  }

  /**
   * @brief The parent raster.
   */
  Parent& m_raster;

  /**
   * @brief The beginning of the rows in the region.
   */
  Box<Dimension> m_beginPlane;

  /**
   * @brief An iterator to m_beginPlane.
   */
  typename Box<Dimension>::Iterator m_beginIt;

  /**
   * @brief The size of the rows.
   */
  Index m_width;

  /**
   * @brief The current pointer.
   */
  Value* m_current;

  /**
   * @brief The end-of-current-line pointer.
   */
  Value* m_eol;
};

/**
 * @relates Subraster
 * @brief Iterator to the beginning of a subraster.
 */
template <typename TRaster, typename T>
typename Subraster<TRaster, T>::Iterator begin(const Subraster<TRaster, T>& subraster) {
  return typename Subraster<TRaster, T>::Iterator(subraster, subraster.region().front());
}

/**
 * @relates Subraster
 * @brief Iterator to one past the end of a subraster.
 */
template <typename TRaster, typename T>
typename Subraster<TRaster, T>::Iterator end(const Subraster<TRaster, T>& subraster) {
  auto endPosition = subraster.region().front();
  --endPosition[0];
  return typename Subraster<TRaster, T>::Iterator(subraster, endPosition);
}

} // namespace Litl

#endif
