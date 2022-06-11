// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_HOLDER_H
#define _RASTER_HOLDER_H

#include <algorithm> // copy_n
#include <array>
#include <cstddef> // size_t
#include <exception> // runtime_error

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

} // namespace Cnes

#endif
