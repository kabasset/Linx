// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_IMPL_PROFILEGENERATOR_H
#define _LINXDATA_IMPL_PROFILEGENERATOR_H

#include "Linx/Data/Tiling.h"

namespace Linx {

/// @cond
namespace Internal {

template <typename TParent, Index I>
using ProfileGeneratorValueImpl =
    decltype(TParent().template profile<I>(Position<(TParent::Dimension == -1 ? -1 : TParent::Dimension - 1)>()));

// FIXME GeneratorMixin
template <typename TParent, Index I>
class ProfileGenerator : public std::iterator<std::forward_iterator_tag, ProfileGeneratorValueImpl<TParent, I>> {
public:

  using Value = ProfileGeneratorValueImpl<TParent, I>;
  static constexpr Index Dimension = TParent::Dimension == -1 ? -1 : TParent::Dimension - 1;

  ProfileGenerator(TParent& in) : m_parent(&in), m_fronts(erase<I>(m_parent->domain())), m_current(m_fronts.begin()) {}

  ProfileGenerator begin() const
  {
    return *this;
  }

  ProfileGenerator end() const
  {
    return ProfileGenerator(m_fronts.end());
  }

  Value operator*() const
  {
    return (*m_parent).template profile<I>(*m_current);
  }

  Value* operator->() const
  {
    return &*(*this);
  }

  ProfileGenerator& operator++()
  {
    ++m_current;
    return *this;
  }

  ProfileGenerator operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  bool operator==(const ProfileGenerator& rhs) const
  {
    return m_current == rhs.m_current;
  }

  bool operator!=(const ProfileGenerator& rhs) const
  {
    return m_current != rhs.m_current;
  }

  Raster<std::decay_t<Value>, Dimension> raster() const
  {
    return Raster<std::decay_t<Value>, Dimension>(m_fronts.shape(), *this);
  }

private:

  ProfileGenerator(typename Box<Dimension>::Iterator current) :
      m_parent(nullptr), m_fronts(), m_current(current) {} // FIXME no copy

  TParent* m_parent;
  Box<Dimension> m_fronts;
  typename Box<Dimension>::Iterator m_current;
};

} // namespace Internal
/// @endcond

} // namespace Linx

#endif
