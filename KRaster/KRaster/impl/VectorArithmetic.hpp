// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#if defined(_KRASTER_VECTORARITHMETIC_IMPL) || defined(CHECK_QUALITY)

#include "KRaster/DataUtils.h"
#include "KRaster/VectorArithmetic.h"

namespace Cnes {

template <typename T, typename TDerived, bool Incrementable>
TDerived VectorArithmeticMixin<T, TDerived, Incrementable>::operator+() const {
  return *this; // Copies
}

template <typename T, typename TDerived, bool Incrementable>
TDerived VectorArithmeticMixin<T, TDerived, Incrementable>::operator-() const {
  TDerived res = static_cast<const TDerived&>(*this);
  std::transform(res.begin(), res.end(), res.begin(), [&](auto r) {
    return -r;
  });
  return res;
}

template <typename T, typename TDerived, bool Incrementable>
TDerived& VectorArithmeticMixin<T, TDerived, Incrementable>::operator+=(const TDerived& rhs) {
  std::transform(
      static_cast<TDerived*>(this)->begin(),
      static_cast<TDerived*>(this)->end(),
      rhs.begin(),
      static_cast<TDerived*>(this)->begin(),
      std::plus<>());
  return static_cast<TDerived&>(*this);
}

template <typename T, typename TDerived, bool Incrementable>
TDerived& VectorArithmeticMixin<T, TDerived, Incrementable>::operator+=(const T& rhs) {
  std::transform(
      static_cast<TDerived*>(this)->begin(),
      static_cast<TDerived*>(this)->end(),
      static_cast<TDerived*>(this)->begin(),
      [&](auto lhs) {
        return lhs + rhs;
      });
  return static_cast<TDerived&>(*this);
}

template <typename T, typename TDerived, bool Incrementable>
TDerived& VectorArithmeticMixin<T, TDerived, Incrementable>::operator-=(const TDerived& rhs) {
  std::transform(
      static_cast<TDerived*>(this)->begin(),
      static_cast<TDerived*>(this)->end(),
      rhs.begin(),
      static_cast<TDerived*>(this)->begin(),
      std::minus<>());
  return static_cast<TDerived&>(*this);
}

template <typename T, typename TDerived, bool Incrementable>
TDerived& VectorArithmeticMixin<T, TDerived, Incrementable>::operator-=(const T& rhs) {
  std::transform(
      static_cast<TDerived*>(this)->begin(),
      static_cast<TDerived*>(this)->end(),
      static_cast<TDerived*>(this)->begin(),
      [&](auto lhs) {
        return lhs - rhs;
      });
  return static_cast<TDerived&>(*this);
}

template <typename T, typename TDerived, bool Incrementable>
TDerived& VectorArithmeticMixin<T, TDerived, Incrementable>::operator*=(const T& rhs) {
  std::transform(
      static_cast<TDerived*>(this)->begin(),
      static_cast<TDerived*>(this)->end(),
      static_cast<TDerived*>(this)->begin(),
      [&](auto lhs) {
        return lhs * rhs;
      });
  return static_cast<TDerived&>(*this);
}

template <typename T, typename TDerived, bool Incrementable>
TDerived& VectorArithmeticMixin<T, TDerived, Incrementable>::operator/=(const T& rhs) {
  std::transform(
      static_cast<TDerived*>(this)->begin(),
      static_cast<TDerived*>(this)->end(),
      static_cast<TDerived*>(this)->begin(),
      [&](auto lhs) {
        return lhs / rhs;
      });
  return static_cast<TDerived&>(*this);
}

template <typename T, typename TDerived, bool Incrementable>
template <typename TFunc, typename... TContainers>
TDerived& VectorArithmeticMixin<T, TDerived, Incrementable>::generate(TFunc&& func, const TContainers&... args) {
  auto its = std::make_tuple(args.begin()...);
  for (auto& v : static_cast<TDerived&>(*this)) {
    v = iteratorTupleApply(its, func);
  }
  return static_cast<TDerived&>(*this);
}

template <typename T, typename TDerived, bool Incrementable>
template <typename TFunc, typename... TContainers>
TDerived& VectorArithmeticMixin<T, TDerived, Incrementable>::apply(TFunc&& func, const TContainers&... args) {
  return generate(std::forward<TFunc>(func), static_cast<TDerived&>(*this), args...);
}

template <typename T, typename TDerived>
TDerived& VectorArithmeticMixin<T, TDerived, true>::operator++() {
  std::transform(
      static_cast<TDerived*>(this)->begin(),
      static_cast<TDerived*>(this)->end(),
      static_cast<TDerived*>(this)->begin(),
      [](auto rhs) {
        return ++rhs;
      });
  return static_cast<TDerived&>(*this);
}

template <typename T, typename TDerived>
TDerived& VectorArithmeticMixin<T, TDerived, true>::operator--() {
  std::transform(
      static_cast<TDerived*>(this)->begin(),
      static_cast<TDerived*>(this)->end(),
      static_cast<TDerived*>(this)->begin(),
      [](auto rhs) {
        return --rhs;
      });
  return static_cast<TDerived&>(*this);
}

} // namespace Cnes

#endif
