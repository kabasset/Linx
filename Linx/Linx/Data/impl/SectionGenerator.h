// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_IMPL_SECTIONGENERATOR_H
#define _LINXDATA_IMPL_SECTIONGENERATOR_H

#include "Linx/Data/Tiling.h"

namespace Linx {

/// @cond
namespace Internal {

template <typename TParent>
class ChunkGenerator : public std::iterator<std::forward_iterator_tag, decltype(TParent().chunk(0, 0))> {
public:

  using Value = decltype(TParent().chunk(0, 0));

  ChunkGenerator(TParent& in, Index thickness) :
      m_parent(&in), m_size(m_parent->length(TParent::Dimension - 1)), m_thickness(thickness), m_current(0),
      m_next(thickness)
  {}

  ChunkGenerator begin() const
  {
    return *this;
  }

  ChunkGenerator end() const
  {
    return ChunkGenerator(m_size);
  }

  Value operator*() const // FIXME return Value&?
  {
    return m_parent->chunk(m_current, m_next - 1);
  }

  Value* operator->() const
  {
    return &*(*this);
  }

  ChunkGenerator& operator++()
  {
    m_current = m_next;
    m_next = std::min(m_size, m_next + m_thickness);
    return *this;
  }

  ChunkGenerator operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  bool operator==(const ChunkGenerator& rhs) const
  {
    return m_current == rhs.m_current;
  }

  bool operator!=(const ChunkGenerator& rhs) const
  {
    return m_current != rhs.m_current;
  }

  Raster<std::decay_t<Value>, 1> raster() const
  {
    return Raster<std::decay_t<Value>, 1>({(m_size + m_thickness - 1) / m_thickness}, *this);
  }

private:

  ChunkGenerator(Index front) : m_parent(nullptr), m_size(0), m_thickness(0), m_current(front), m_next(0) {}

  TParent* m_parent;
  Index m_size;
  Index m_thickness;
  Index m_current;
  Index m_next;
};

template <typename TParent>
class SectionGenerator : public std::iterator<std::forward_iterator_tag, decltype(TParent().section(0))> {
public:

  using Value = decltype(TParent().section(0));

  SectionGenerator(TParent& in) : m_parent(&in), m_size(m_parent->length(TParent::Dimension - 1)), m_current(0) {}

  SectionGenerator begin() const
  {
    return *this;
  }

  SectionGenerator end() const
  {
    return SectionGenerator(m_size);
  }

  Value operator*() const // FIXME return Value&?
  {
    return m_parent->section(m_current);
  }

  Value* operator->() const
  {
    return &*(*this);
  }

  SectionGenerator& operator++()
  {
    ++m_current;
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

  Raster<std::decay_t<Value>, 1> raster() const
  {
    return Raster<std::decay_t<Value>, 1>({m_size}, *this);
  }

private:

  SectionGenerator(Index front) : m_parent(nullptr), m_size(0), m_current(front) {}

  TParent* m_parent;
  Index m_size;
  Index m_current;
};

} // namespace Internal
/// @endcond

} // namespace Linx

#endif
