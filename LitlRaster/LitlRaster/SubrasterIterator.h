// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_SUBRASTERITERATOR_H
#define _LITLRASTER_SUBRASTERITERATOR_H

#include "LitlRaster/Subraster.h"

namespace Litl {

template <typename T, typename TRaster, typename TRegion>
template <typename U>
class Subraster<T, TRaster, TRegion>::Iterator : public std::iterator<std::input_iterator_tag, U> {

public:
  using Parent = TRaster;
  using Value = U;
  static constexpr Index Dimension = TRaster::Dimension;

  Iterator(TRaster& raster, const Box<Dimension>& region, const Position<Dimension>& position) :
      m_raster(raster), m_beginPlane(project(region)), m_beginIt(m_beginPlane, position), m_width(region.length(0)),
      m_current(&m_raster[position]), m_eol(m_current + m_width) {}

  static Iterator begin(TRaster& raster, const TRegion& region) {
    return Iterator(raster, region, Box<Dimension>::Iterator::beginPosition(region));
  }

  static Iterator end(TRaster& raster, const TRegion& region) {
    return Iterator(raster, region, Box<Dimension>::Iterator::endPosition(region));
  }

  Value& operator*() const {
    return *m_current;
  }

  Value* operator->() const {
    return m_current;
  }

  Value& operator++() {
    return *next();
  }

  Value* operator++(int) {
    return next();
  }

  bool operator==(const Iterator& rhs) const {
    return m_current == rhs.m_current;
  }

  bool operator!=(const Iterator& rhs) const {
    return m_current != rhs.m_current;
  }

private:
  inline Value* next() {
    if (++m_current != m_eol) {
      return m_current;
    }
    ++m_beginIt;
    // Use data() and index() instead of [] to avoid dereferencing unallocated memory
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

} // namespace Litl

#endif
