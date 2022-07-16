// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLCONTAINER_SEQUENCE_H
#define _LITLCONTAINER_SEQUENCE_H

#include "LitlContainer/Arithmetic.h"
#include "LitlContainer/DataContainer.h"
#include "LitlContainer/Holders.h"
#include "LitlContainer/Math.h"
#include "LitlContainer/Random.h"

namespace Litl {

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

/**
 * @relates Sequence
 * @brief Generate a random raster.
 * @details
 * Pixel values are uniformly distributed between the type's half min and half max.
 */
template <typename T>
Sequence<T> random(std::size_t size) {
  Sequence<T> out(size);
  out.generate(UniformNoise<T>());
  return out;
}

} // namespace Litl

#endif
