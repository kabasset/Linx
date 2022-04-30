// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#if defined(_KRASTER_POSITION_IMPL) || defined(CHECK_QUALITY)

#include "KRaster/Position.h"

namespace Cnes {

template <Index N>
Position<N>::Position() : DataContainer<Index, DataContainerHolder<Index, Indices<N>>, Position<N>>() {}

template <Index N>
Position<N>::Position(Index dim) : DataContainer<Index, DataContainerHolder<Index, Indices<N>>, Position<N>>(dim) {}

template <Index N>
Position<N>::Position(std::initializer_list<Index> indices) :
    DataContainer<Index, DataContainerHolder<Index, Indices<N>>, Position<N>>(indices) {}

template <Index N>
template <typename TIterator>
Position<N>::Position(TIterator begin, TIterator end) :
    DataContainer<Index, DataContainerHolder<Index, Indices<N>>, Position<N>>(begin, end) {}

template <Index N>
Position<N> Position<N>::zero() {
  Position<N> res(std::abs(Dim));
  std::fill(res.begin(), res.end(), 0);
  return res;
}

template <Index N>
Position<N> Position<N>::one() {
  Position<N> res(std::abs(Dim));
  std::fill(res.begin(), res.end(), 1);
  return res;
}

template <Index N>
Position<N> Position<N>::max() {
  Position<N> res(std::abs(N));
  std::fill(res.begin(), res.end(), -1);
  return res;
}

template <Index N>
bool Position<N>::isZero() const {
  for (auto i : *this) {
    if (i != 0) {
      return false;
    }
  }
  return true;
}

template <Index N>
bool Position<N>::isMax() const {
  for (auto i : *this) {
    if (i != -1) {
      return false;
    }
  }
  return true;
}

template <Index N>
template <Index M>
Position<M> Position<N>::slice() const {
  const auto b = this->begin();
  auto e = b;
  std::advance(e, M);
  return Position<M>(b, e);
}

template <Index N>
template <Index M>
Position<M> Position<N>::extend(const Position<M>& padding) const {
  auto res = padding;
  for (std::size_t i = 0; i < this->size(); ++i) { // TODO std::transform
    res[i] = (*this)[i];
  }
  return res;
}

} // namespace Cnes

#endif
