// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_DATACONTAINER_H
#define _RASTER_DATACONTAINER_H

#include "Raster/ContiguousContainer.h"
#include "Raster/DataDistribution.h"
#include "Raster/Holder.h"
#include "RasterTypes/Exceptions.h"
#include "RasterTypes/SeqUtils.h" // isIterable
#include "RasterTypes/TypeUtils.h" // Index, Limits

#include <cstddef> // size_t
#include <initializer_list>
#include <tuple>
#include <type_traits> // enable_if, is_integral, decay
#include <utility> // forward

namespace Cnes {

/**
 * @ingroup data_classes
 * @brief Combine a `ContiguousContainerMixin` and `DataHolder` as a user-defined derived class.
 * @tparam T The element type
 * @tparam THolder The data container holder
 * @tparam TDerived The derived class
 * @details
 * The class can be specialized for any container which implements the `SizedData` requirements.
 * @satisfies{ContiguousContainer}
 */
template <typename T, typename THolder, typename TDerived>
class DataContainer : public ContiguousContainerMixin<T, TDerived>, public THolder {

public:
  /**
   * @brief The concrete data holder type.
   */
  using Holder = THolder;

  /// @{
  /// @group_construction

  /**
   * @brief Size-based constructor.
   */
  template <typename... TArgs>
  explicit DataContainer(std::size_t s = 0, TArgs&&... args) : Holder(s, std::forward<TArgs>(args)...) {}

  /**
   * @brief List-based constructor.
   * @details
   * List values are copied to the container.
   */
  template <typename U, typename... TArgs>
  explicit DataContainer(std::initializer_list<U> list, TArgs&&... args) :
      Holder(list.size(), std::forward<TArgs>(args)...) {
    std::copy(list.begin(), list.end(), this->data());
  }

  /**
   * @brief Iterator-based constructor.
   * @details
   * Iterable values are copied to the container.
   */
  template <typename TIterable, typename std::enable_if_t<isIterable<TIterable>::value>* = nullptr, typename... TArgs>
  explicit DataContainer(const TIterable& iterable, TArgs&&... args) :
      Holder(std::distance(iterable.begin(), iterable.end()), std::forward<TArgs>(args)...) {
    std::copy(iterable.begin(), iterable.end(), this->data());
  }

  CNES_VIRTUAL_DTOR(DataContainer)
  CNES_DEFAULT_COPYABLE(DataContainer)
  CNES_DEFAULT_MOVABLE(DataContainer)

  /// @group_properties

  /**
   * @brief Inherit data holder's `size()`.
   */
  using Holder::size;

  /// @group_elements

  /**
   * @brief Inherit data holder's `data()`.
   */
  using Holder::data;

  /**
   * @brief Access the raw data.
   */
  inline T* data() {
    return const_cast<T*>(const_cast<const DataContainer&>(*this).data());
  }

  /**
   * @brief Access the element with given index.
   * @details
   * As opposed to `operator[]()`, negative indices are supported for backward indexing,
   * and bounds are checked.
   */
  const T& at(Index i) const {
    const auto s = size();
    OutOfBoundsError::mayThrow("Index " + std::to_string(i), i, {-s, s - 1});
    return this->operator[](i >= 0 ? i : i + s);
  }

  /**
   * @copybrief at()
   */
  T& at(Index i) {
    return const_cast<T&>(const_cast<const DataContainer&>(*this).at(i));
  }

  /// @group_modifiers

  /**
   * @brief Fill the container with a single value.
   */
  TDerived& fill(const T& value) {
    std::fill(this->begin(), this->end(), value);
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief Fill the container with evenly spaced value.
   * @details
   * The difference between two adjacent values is _exactly_ `step`,
   * i.e. `container[i + 1] = container[i] + step`.
   * This means that rounding errors may sum up,
   * as opposed to `linspace()`.
   * @see `linspace()`
   */
  TDerived& range(const T& min = Limits<T>::zero(), const T& step = Limits<T>::one()) {
    auto v = min;
    auto& t = static_cast<TDerived&>(*this);
    for (auto& e : t) {
      e = v;
      v += step;
    }
    return t;
  }

  /**
   * @brief Fill the container with evenly spaced value.
   * @details
   * The first and last values of the container are _exactly_ `min` and `max`.
   * Intermediate values are computed as `container[i] = min + (max - min) / (size() - 1) * i`,
   * which means that the difference between two adjacent values
   * is not necessarily perfectly constant for floating point values,
   * as opposed to `range()`.
   * @see `range()`
   */
  TDerived& linspace(const T& min = Limits<T>::zero(), const T& max = Limits<T>::one()) {
    const auto step = (max - min) / (this->size() - 1);
    auto it = this->begin();
    for (std::size_t i = 0; i < this->size() - 1; ++i, ++it) {
      *it = min + step * i;
    }
    *it = max;
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief Generate values from a function with optional input containers.
   * @param func The generator function, which takes as many inputs as there are arguments
   * @param args The arguments in the form of containers of compatible sizes
   * @details
   * For example, here is how to imlement element-wise square root and multiplication:
   * \code
   * Container a = ...;
   * Container b = ...;
   * Container res(a.size());
   * res.generate([](auto v) { return std::sqrt(v) }, a); // res = sqrt(a)
   * res.generate([](auto v, auto w) { return v * w; }, a, b); // res = a * b
   * \endcode
   */
  template <typename TFunc, typename... TContainers>
  TDerived& generate(TFunc&& func, const TContainers&... args) {
    auto its = std::make_tuple(args.begin()...);
    auto& t = static_cast<TDerived&>(*this);
    for (auto& v : t) {
      v = iteratorTupleApply(its, func);
    }
    return t;
  }

  /**
   * @brief Apply a function with optional input containers.
   * @param func The function
   * @param args The arguments in the form of containers of compatible sizes
   * @details
   * If there are _n_ arguments, `func` takes _n_+1 parameters,
   * where the first argument is the element of this container.
   * For example, here is how to imlement in-place element-wise square root and multiplication:
   * \code
   * Container a = ...;
   * Container res = ...;
   * res.apply([](auto v) { return std::sqrt(v); }); // res = sqrt(res)
   * res.apply([](auto v, auto w) { return v * w; }, a); // res *= a
   * \endcode
   */
  template <typename TFunc, typename... TContainers>
  TDerived& apply(TFunc&& func, const TContainers&... args) {
    return generate(std::forward<TFunc>(func), static_cast<TDerived&>(*this), args...);
  }

  /// @group_operators

  /**
   * @brief Get a reference to the (first) min element.
   * @see `distribution()`
   */
  const T& min() const {
    return *std::min_element(this->begin(), this->end());
  }

  /**
   * @brief Get a reference to the (first) max element.
   * @see `distribution()`
   */
  const T& max() const {
    return *std::max_element(this->begin(), this->end());
  }

  /**
   * @brief Get a pair of references to the (first) min and max elements.
   * @see `distribution()`
   */
  std::pair<const T&, const T&> minmax() const {
    const auto its = std::minmax_element(this->begin(), this->end());
    return {*its.first, *its.second};
  }

  /**
   * @brief Create a `DataDistribution` from the container.
   */
  DataDistribution<T> distribution() const {
    return DataDistribution<T>(*this);
  }

  /// @}
};

} // namespace Cnes

#endif
