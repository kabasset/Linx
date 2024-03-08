// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXDATA_IMPL_ROWGENERATOR_H
#define _LINXDATA_IMPL_ROWGENERATOR_H

#include "Linx/Data/Tiling.h"

namespace Linx {

/// @cond
namespace Internal {

template <typename TParent>
class RowGenerator :
    public std::iterator<
        std::forward_iterator_tag,
        decltype(TParent().row(Position<TParent::OneLessDimension>()))> { // FIXME Dimensional?
public:

  static constexpr Index Dimension = TParent::OneLessDimension;
  using Value = decltype(TParent().row(Position<Dimension>()));

  RowGenerator(TParent& in) : m_parent(&in), m_fronts(erase<0>(in.domain())), m_current(m_fronts.begin()) {}

  RowGenerator begin() const
  {
    return *this;
  }

  RowGenerator end() const
  {
    return RowGenerator(m_fronts.end());
  }

  Value operator*() const
  {
    return m_parent->row(*m_current);
  }

  Value* operator->() const
  {
    return &*(*this);
  }

  RowGenerator& operator++()
  {
    ++m_current;
    return *this;
  }

  RowGenerator operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  bool operator==(const RowGenerator& rhs) const
  {
    return m_current == rhs.m_current;
  }

  bool operator!=(const RowGenerator& rhs) const
  {
    return m_current != rhs.m_current;
  }

  Raster<std::decay_t<Value>, Dimension> raster() const
  {
    return Raster<std::decay_t<Value>, Dimension>(m_fronts.shape(), *this);
  }

private:

  RowGenerator(const typename Box<Dimension>::Iterator& current) :
      m_parent(nullptr), m_fronts(), m_current(current) {} // FIXME no copy

  TParent* m_parent;
  Box<Dimension> m_fronts;
  typename Box<Dimension>::Iterator m_current;
};

} // namespace Internal
/// @endcond

} // namespace Linx

#endif
