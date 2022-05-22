// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_DATACONTAINER_H
#define _RASTER_DATACONTAINER_H

#include "Raster/ContiguousContainer.h"
#include "Raster/DataUtils.h"
#include "Raster/VectorArithmetic.h"
#include "RasterTypes/SeqUtils.h" // isIterable

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
 * The class can be specialized for any container,
 * in which case it should satisfy the `SizedData` requirements.
 * @satisfies{SizedData}
 */
template <typename T, typename TContainer>
class DataContainerHolder {

public:
  /**
   * @brief The concrete container type.
   */
  using Container = TContainer;

  /// @group_construction

  /**
   * @brief Default or size-based constructor.
   */
  template <typename U>
  explicit DataContainerHolder(std::size_t size, U* data = nullptr) : m_container(size) {
    if (data) {
      std::copy_n(data, size, const_cast<T*>(this->data()));
    }
  }

  /**
   * @brief Forwarding constructor.
   */
  template <typename... TArgs>
  explicit DataContainerHolder(TArgs&&... args) : m_container(std::forward<TArgs>(args)...) {}

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
  inline const T* data() const {
    return m_container.data();
  }

  /// @}

protected:
  /**
   * @brief The underlying container.
   */
  Container m_container;
};

/**
 * @ingroup data_classes
 * @brief Raw pointer specialization.
 * @details
 * This is a non-owning holder, i.e. some view on exising data.
 * No memory is managed.
 */
template <typename T>
class DataContainerHolder<T, T*> {

public:
  using Container = T*;

  explicit DataContainerHolder(std::size_t size, T* data) : m_size(size), m_container(data) {}

  std::size_t size() const {
    return m_size;
  }

  inline const T* data() const {
    return m_container;
  }

protected:
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
 * @brief `std::array` specialization.
 */
template <typename T, std::size_t N>
class DataContainerHolder<T, std::array<T, N>> {

public:
  using Container = std::array<T, N>;

  explicit DataContainerHolder(std::size_t size, const T* data = nullptr) : m_container {} {
    if (size != N && size != 0) { // FIXME allow size < N? => add m_size
      std::string msg = "Size mismatch in DataContainerHolder<std::array> specialization. ";
      msg += "Got " + std::to_string(size) + ", should be 0 or " + std::to_string(N) + ".";
      throw std::runtime_error(msg);
    }
    if (data) {
      std::copy_n(data, size, m_container.begin());
    }
  }

  std::size_t size() const {
    return N;
  }

  inline const T* data() const {
    return m_container.data();
  }

protected:
  /**
   * @brief The data array.
   */
  std::array<T, N> m_container;
};

/**
 * @ingroup data_classes
 * @brief Mix `ContiguousContainerMixin` and `VectorArithmeticMixin` into a `DataHolder`
 * as a user-defined derived class.
 * @tparam TDerived The derived class
 * @details
 * The class can be specialized for any container which implements the `SizedData` requirements.
 * @satisfies{ContiguousContainer}
 * @satisfies{VectorArithmetic}
 */
template <typename T, typename THolder, typename TDerived> // TODO allow void
class DataContainer :
    public ContiguousContainerMixin<T, TDerived>, // TODO fallback to DataContainer<T, THolder, void>
    public VectorArithmeticMixin<T, TDerived>, // TODO fallback to DataContainer<T, THolder, void>
    public THolder {

public:
  /**
   * @brief The concrete data holder type.
   */
  using Holder = THolder;

  /**
   * @brief The concrete container type.
   */
  using Container = typename Holder::Container;

  /// @group_construction

  /**
   * @brief Size-based constructor.
   */
  template <typename... TArgs>
  explicit DataContainer(std::size_t s = 0, TArgs&&... args) : Holder(s, std::forward<TArgs>(args)...) {}

  /**
   * @brief Iterator-based constructor.
   * @details
   * Iterable values are copied to the container.
   */
  template <typename TIterable, typename std::enable_if_t<isIterable<TIterable>::value>* = nullptr, typename... TArgs>
  explicit DataContainer(TIterable&& iterable, TArgs&&... args) :
      Holder(std::distance(iterable.begin(), iterable.end()), std::forward<TArgs>(args)...) {
    std::copy(iterable.begin(), iterable.end(), this->data());
  }

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

  CNES_VIRTUAL_DTOR(DataContainer)
  CNES_COPYABLE(DataContainer)
  CNES_MOVABLE(DataContainer)

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
   * @copybrief data()const
   */
  inline T* data() {
    return const_cast<T*>(const_cast<const DataContainer&>(*this).data());
  }

  /**
   * @brief Access the container in read-only mode.
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
};

} // namespace Cnes

#endif
