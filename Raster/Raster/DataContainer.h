// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_DATACONTAINER_H
#define _RASTER_DATACONTAINER_H

#include "Raster/ContiguousContainer.h"
#include "Raster/DataUtils.h"
#include "RasterTypes/Exceptions.h"
#include "RasterTypes/SeqUtils.h" // isIterable
#include "RasterTypes/TypeUtils.h" // Limits

#include <algorithm> // copy_n
#include <array>
#include <cstddef> // size_t
#include <exception> // runtime_error
#include <initializer_list>
#include <type_traits> // enable_if, is_integral, decay

namespace Cnes {

/**
 * @ingroup data_classes
 * @brief A default holder of any contiguous container specified by a size and data pointer.
 * @details
 * The class can be specialized for any continuous container,
 * in which case the specialization should satisfy the `SizedData` requirements.
 * @satisfies{SizedData}
 */
template <typename TContainer>
class StdHolder {

public:
  /**
   * @brief The concrete container type.
   */
  using Container = TContainer;

  /// @{
  /// @group_construction

  /**
   * @brief Default or size-based constructor.
   */
  template <typename U = typename TContainer::value_type>
  explicit StdHolder(std::size_t size, U* data = nullptr) : m_container(size) {
    if (data) {
      std::copy_n(data, size, const_cast<typename TContainer::value_type*>(this->data()));
    }
  }

  /**
   * @brief Container-move constructor.
   */
  explicit StdHolder(std::size_t size, Container&& container) : m_container(std::move(container)) {
    // FIXME check size
  }

  /// @group_properties

  /**
   * @brief Get the number of elements.
   */
  std::size_t size() const {
    return m_container.size();
  }

  /// @group_elements

  /**
   * @brief Access the raw data.
   */
  inline const typename TContainer::value_type* data() const {
    return &m_container[0]; // m_container.data() not available for valarray
  }

  /**
   * @brief Access the underlying container in read-only mode.
   */
  const std::decay_t<Container>& container() const {
    return this->m_container;
  }

  /// @group_modifiers

  /**
   * @brief Move the container.
   * @details
   * This method is used to take ownership on the data without copying it.
   * Example usage:
   * \code
   * VecRaster<float> raster(...);
   * std::vector<float> data;
   * raster.moveTo(data);
   * // Values have been moved to data without copy.
   * // raster.vector() is in an unspecified state now.
   * \endcode
   * @warning
   * The container is not usable anymore after this call.
   */
  Container& moveTo(Container& destination) {
    destination = std::move(this->m_container);
    return destination;
  }

  /// @}

private:
  /**
   * @brief The underlying container.
   */
  Container m_container;
};

/**
 * @ingroup data_classes
 * @brief `std::array` specialization.
 */
template <typename T, std::size_t N>
class StdHolder<std::array<T, N>> {

public:
  using Container = std::array<T, N>;

  explicit StdHolder(std::size_t size, const T* data = nullptr) : m_container {} {
    if (size != N && size != 0) { // TODO allow size < N? => add m_size
      std::string msg = "Size mismatch in StdHolder<std::array> specialization. ";
      msg += "Got " + std::to_string(size) + ", should be 0 or " + std::to_string(N) + ".";
      throw std::runtime_error(msg);
    }
    if (data) {
      std::copy_n(data, size, m_container.begin());
    }
  }

  explicit StdHolder(std::size_t size, Container&& container) : m_container(std::move(container)) {
    // FIXME check size
  }

  std::size_t size() const {
    return N;
  }

  inline const T* data() const {
    return m_container.data();
  }

  const std::decay_t<Container>& container() const {
    return this->m_container;
  }

  Container& moveTo(Container& destination) {
    destination = std::move(this->m_container);
    return destination;
  }

private:
  /**
   * @brief The data array.
   */
  std::array<T, N> m_container;
};

/**
 * @ingroup data_classes
 * @brief Raw pointer holder.
 * @details
 * This is a non-owning holder, i.e. some view on exising data.
 * No memory is managed.
 */
template <typename T>
class PtrHolder {

public:
  explicit PtrHolder(std::size_t size, T* data) : m_size(size), m_container(data) {}

  std::size_t size() const {
    return m_size;
  }

  inline const T* data() const {
    return m_container;
  }

private:
  /**
   * @brief The number of elements.
   */
  std::size_t m_size;

  /**
   * @brief The raw data pointer.
   */
  T* m_container;
};

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
  const T& at(long i) const { // FIXME Index
    const auto s = size();
    OutOfBoundsError::mayThrow("Index " + std::to_string(i), i, {-s, s - 1});
    return this->operator[](i >= 0 ? i : i + s);
  }

  /**
   * @copybrief at()
   */
  T& at(long i) { // FIXME Index
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
  TDerived& arange(const T& min = Limits<T>::zero(), const T& step = Limits<T>::one()) {
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
   * as opposed to `arange()`.
   * @see `arange()`
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

  /// @}
};

} // namespace Cnes

#endif
