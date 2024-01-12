// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_SEQUENCE_H
#define _LINXDATA_SEQUENCE_H

#include "Linx/Base/Holders.h"
#include "Linx/Base/Random.h"
#include "Linx/Base/mixins/Arithmetic.h"
#include "Linx/Base/mixins/DataContainer.h"
#include "Linx/Base/mixins/Math.h"

namespace Linx {

template <typename T, typename THolder = DefaultHolder<T>>
class Sequence : public DataContainer<T, THolder, EuclidArithmetic, Sequence<T, THolder>> {
public:

  using Container = DataContainer<T, THolder, EuclidArithmetic, Sequence>;

  template <typename... TArgs>
  explicit Sequence(std::size_t size = 0, TArgs&&... args) : Container(size, std::forward<TArgs>(args)...)
  {}

  Sequence(std::initializer_list<T> list) : Container(std::move(list)) {}

  template <typename... TArgs>
  explicit Sequence(std::initializer_list<T> list, TArgs&&... args) :
      Container(list.begin(), list.end(), std::forward<TArgs>(args)...)
  {}

  template <typename TRange, typename std::enable_if_t<IsRange<TRange>::value>* = nullptr, typename... TArgs>
  explicit Sequence(const TRange& range, TArgs&&... args) :
      Container(range.begin(), range.end(), std::forward<TArgs>(args)...)
  {}
};

/**
 * @relatesalso Sequence
 * @brief `Sequence` which owns a `std::array`.
 */
template <typename T, std::size_t Capacity>
using ArrSequence = Sequence<T, StdHolder<std::array<T, Capacity>>>;

/**
 * @relatesalso Sequence
 * @brief Generate a random raster.
 * 
 * Pixel values are uniformly distributed between the type's half min and half max.
 */
template <typename T>
Sequence<T> random(std::size_t size)
{
  Sequence<T> out(size);
  out.generate(UniformNoise<T>());
  return out;
}

} // namespace Linx

#endif
