// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXCORE_SEQUENCE_H
#define _LINXCORE_SEQUENCE_H

#include "LinxBase/Arithmetic.h"
#include "LinxBase/DataContainer.h"
#include "LinxBase/Holders.h"
#include "LinxBase/Math.h"
#include "LinxBase/Random.h"

namespace Linx {

template <typename T, typename THolder = DefaultHolder<T>>
class Sequence : public DataContainer<T, THolder, EuclidArithmetic, Sequence<T, THolder>> {
public:
  using Container = DataContainer<T, THolder, EuclidArithmetic, Sequence>;

  template <typename... TArgs>
  explicit Sequence(std::size_t size = 0, TArgs&&... args) : Container(size, std::forward<TArgs>(args)...) {}

  Sequence(std::initializer_list<T> list) : Container(std::move(list)) {}

  template <typename... TArgs>
  explicit Sequence(std::initializer_list<T> list, TArgs&&... args) :
      Container(std::move(list), std::forward<TArgs>(args)...) {}

  template <typename TRange, typename std::enable_if_t<IsRange<TRange>::value>* = nullptr, typename... TArgs>
  explicit Sequence(const TRange& range, TArgs&&... args) : Container(range, std::forward<TArgs>(args)...) {}
};

/**
 * @relates Sequence
 * @brief Generate a random raster.
 * 
 * Pixel values are uniformly distributed between the type's half min and half max.
 */
template <typename T>
Sequence<T> random(std::size_t size) {
  Sequence<T> out(size);
  out.generate(UniformNoise<T>());
  return out;
}

} // namespace Linx

#endif
