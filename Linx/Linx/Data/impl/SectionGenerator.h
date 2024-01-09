// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_IMPL_SECTIONGENERATOR_H
#define _LINXDATA_IMPL_SECTIONGENERATOR_H

#include "Linx/Data/Tiling.h"

namespace Linx {

/// @cond
namespace Internal {

template <typename TParent>
class SectionGenerator : public std::iterator<std::forward_iterator_tag, decltype(TParent().section(0, 0))> {
public:

  using Value = decltype(TParent().section(0, 0));

  SectionGenerator(TParent& in, Index thickness) :
      m_parent(&in), m_size(m_parent->length(TParent::Dimension - 1)), m_thickness(thickness), m_current(0),
      m_next(thickness)
  {}

  SectionGenerator begin() const
  {
    return *this;
  }

  SectionGenerator& begin()
  {
    return *this;
  }

  SectionGenerator end() const
  {
    return SectionGenerator(m_size);
  }

  Value operator*() const // FIXME return Value&?
  {
    return m_parent->section(m_current, m_next - 1);
  }

  Value* operator->() const
  {
    return &*(*this);
  }

  SectionGenerator& operator++()
  {
    m_current = m_next;
    m_next = std::min(m_size, m_next + m_thickness);
    return *this;
  }

  SectionGenerator operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  bool operator==(const SectionGenerator& rhs) const
  {
    return m_current == rhs.m_current;
  }

  bool operator!=(const SectionGenerator& rhs) const
  {
    return m_current != rhs.m_current;
  }

  Raster<std::decay_t<Value>, 1> raster()
  {
    return Raster<std::decay_t<Value>, 1>({(m_size + m_thickness - 1) / m_thickness}, *this);
  }

private:

  SectionGenerator(Index front) : m_parent(nullptr), m_current(front), m_next(front) {}

  TParent* m_parent;
  Index m_size;
  Index m_thickness;
  Index m_current;
  Index m_next;
};

} // namespace Internal
/// @endcond

} // namespace Linx

#endif
