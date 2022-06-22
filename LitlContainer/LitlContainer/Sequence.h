// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLCONTAINER_SEQUENCE_H
#define _LITLCONTAINER_SEQUENCE_H

#include "LitlContainer/Arithmetic.h"
#include "LitlContainer/DataContainer.h"
#include "LitlContainer/Holders.h"
#include "LitlContainer/Math.h"
#include "LitlContainer/Random.h"

namespace Cnes {

template <typename T, typename THolder = DefaultHolder<T>>
class Sequence : public DataContainer<T, THolder, EuclidArithmetic, Sequence<T, THolder>> {
public:
  using Container = DataContainer<T, THolder, EuclidArithmetic, Sequence>;

  template <typename... TArgs>
  explicit Sequence(std::size_t size = 0, TArgs&&... args) : Container(size, std::forward<TArgs>(args)...) {}

  template <typename U>
  Sequence(std::initializer_list<U> list) : Container(list) {}

  template <typename U, typename... TArgs>
  explicit Sequence(std::initializer_list<U> list, TArgs&&... args) : Container(list, std::forward<TArgs>(args)...) {}

  template <typename TIterable, typename std::enable_if_t<isIterable<TIterable>::value>* = nullptr, typename... TArgs>
  explicit Sequence(TIterable& iterable, TArgs&&... args) : Container(iterable, std::forward<TArgs>(args)...) {}
};

template <typename T>
Sequence<T> random(std::size_t size) {
  Sequence<T> out(size);
  out.generate(UniformNoise<T>());
  return out;
}

} // namespace Cnes

#endif
