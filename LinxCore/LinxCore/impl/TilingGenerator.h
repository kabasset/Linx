// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXCORE_IMPL_TILINGITERATOR_H
#define _LINXCORE_IMPL_TILINGITERATOR_H

#include "LinxCore/Tiling.h"

namespace Linx {

template <typename TParent, Index M>
class TileGenerator : public std::iterator<std::forward_iterator_tag, decltype(TParent().patch(Box<M>()))> {

public:
  using Value = decltype(TParent().patch(Box<M>()));

  TileGenerator(TParent& in, Position<M> shape) :
      m_parent(&in), m_domain(m_parent->domain()), m_fronts(m_domain, std::move(shape)), m_current(m_fronts.begin()) {}

  TileGenerator& begin() {
    return *this;
  }

  TileGenerator end() const {
    return TileGenerator(m_fronts.end());
  }

  Value operator*() const { // FIXME store patch and return Value&
    return m_parent->patch(Box<M>::fromShape(*m_current, m_fronts.step()) & m_domain);
  }

  Value* operator->() const {
    return &*(*this);
  }

  TileGenerator& operator++() {
    ++m_current;
    return *this;
  }

  TileGenerator operator++(int) {
    auto out = *this;
    ++(*this);
    return out;
  }

  bool operator==(const TileGenerator& rhs) const {
    return m_current == rhs.m_current;
  }

  bool operator!=(const TileGenerator& rhs) const {
    return m_current != rhs.m_current;
  }

private:
  TileGenerator(typename Grid<TParent::Dimension>::Iterator current) :
      m_parent(nullptr), m_domain(), m_fronts(), m_current(current) {} // FIXME no copy

  TParent* m_parent;
  Box<TParent::Dimension> m_domain; // FIXME only back is needed for clamping
  Grid<TParent::Dimension> m_fronts;
  typename Grid<TParent::Dimension>::Iterator m_current;
};

} // namespace Linx

#endif
