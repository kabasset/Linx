// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXDATA_IMPL_TILEGENERATOR_H
#define _LINXDATA_IMPL_TILEGENERATOR_H

#include "Linx/Data/Tiling.h"

namespace Linx {

/// @cond
namespace Internal {

template <typename TParent, Index M>
class TileGenerator : public std::iterator<std::forward_iterator_tag, decltype(std::declval<TParent>()(Box<M>()))> {
public:

  using Value = decltype(std::declval<TParent>()(Box<M>()));
  static constexpr Index Dimension = M;

  TileGenerator(TParent& in, Position<M> shape) :
      m_parent(&in), m_domain(m_parent->domain()), m_fronts(m_domain, std::move(shape)), m_current(m_fronts.begin())
  {}

  TileGenerator begin() const
  {
    return *this;
  }

  TileGenerator end() const
  {
    return TileGenerator(m_fronts.end());
  }

  Value operator*() const
  {
    return (*m_parent)(Box<M>::from_shape(*m_current, m_fronts.step()) & m_domain);
  }

  Value* operator->() const
  {
    return &*(*this);
  }

  TileGenerator& operator++()
  {
    ++m_current;
    return *this;
  }

  TileGenerator operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  bool operator==(const TileGenerator& rhs) const
  {
    return m_current == rhs.m_current;
  }

  bool operator!=(const TileGenerator& rhs) const
  {
    return m_current != rhs.m_current;
  }

  Raster<std::decay_t<Value>, Dimension> raster() const
  {
    return Raster<std::decay_t<Value>, Dimension>(m_fronts.shape(), *this);
  }

private:

  TileGenerator(typename Grid<TParent::Dimension>::Iterator current) :
      m_parent(nullptr), m_domain(), m_fronts(), m_current(current)
  {} // FIXME no copy

  TParent* m_parent;
  Box<TParent::Dimension> m_domain; // FIXME only back is needed for clamping
  Grid<TParent::Dimension> m_fronts;
  typename Grid<TParent::Dimension>::Iterator m_current;
};

} // namespace Internal
/// @endcond

} // namespace Linx

#endif
