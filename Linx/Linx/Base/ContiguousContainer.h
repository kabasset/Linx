// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXBASE_CONTIGUOUSCONTAINER_H
#define _LINXBASE_CONTIGUOUSCONTAINER_H

#include <algorithm> // equal
#include <ostream>

namespace Linx {

/**
 * @ingroup concepts
 * @requirements{ContiguousContainer}
 * @brief Standard contiguous container requirements
 * 
 * A <a href="https://en.cppreference.com/w/cpp/named_req/ContiguousContainer">contiguous container</a>
 * is a standard container whose elements are stored contiguously in memory.
 */

/**
 * @ingroup concepts
 * @requirements{ContiguousRange}
 * @brief Requirements for the holder of a `DataContainer`.
 * 
 * A contiguous data holder is a class which stores or points to some data contiguous in memory,
 * and implements the following methods:
 * - Constructor from a size and pointer;
 * - `inline const T* begin() const`;
 * - `inline const T* end() const`.
 * 
 * @par_example
 * Here is a minimal `ContiguousRange`-compliant class:
 * \snippet LinxDemoBasics_test.cpp MallocRaster
 */

/**
 * @ingroup mixins
 * @brief Base class for a contiguous container.
 * 
 * @tparam T The value type
 * @tparam TDerived The child class which implements required methods
 * 
 * 
 * This class provides the necessary types and methods
 * to meet the standard `ContiguousContainer` requirements.
 * This is a CRTP implementation, which means it takes as template parameter
 * the derived class to be empowered.
 * 
 * The derived class must satisfy the `ContiguousRange` requirements.
 * 
 * @satisfies{ContiguousContainer}
 */
template <typename T, typename TDerived>
struct ContiguousContainerMixin {

  /**
   * @brief The value type.
   */
  using value_type = T;

  /**
   * @brief The value reference.
   */
  using reference = T&;

  /**
   * @brief The constant value reference.
   */
  using const_reference = const T&;

  /**
   * @brief The value iterator.
   */
  using iterator = T*;

  /**
   * @brief The constant value iterator.
   */
  using const_iterator = const T*;

  /**
   * @brief The iterator difference type.
   */
  using difference_type = std::ptrdiff_t;

  /**
   * @brief The underlying container size type.
   */
  using size_type = std::size_t;

  /// @{
  /// @group_properties

  /**
   * @brief Check whether the container is empty.
   * 
   * Empty corresponds to `begin() == end()`.
   */
  bool empty() const {
    return static_cast<const TDerived&>(*this).end() == static_cast<const TDerived&>(*this).begin();
  }

  /**
   * @brief Number of elements.
   */
  std::size_t size() const {
    return static_cast<const TDerived&>(*this).end() - static_cast<const TDerived&>(*this).begin();
  }

  /**
   * @brief Maximum number of elements the container is able to hold (same as `size()`).
   */
  std::size_t max_size() const {
    return size();
  }

  /// @group_elements

  /**
   * @brief Get a pointer to the underlying array.
   */
  inline const T* data() const {
    return static_cast<const TDerived&>(*this).begin();
  }

  /**
   * @copybrief data()const
   */
  inline T* data() {
    return const_cast<T*>(const_cast<const ContiguousContainerMixin&>(*this).data());
  }

  /**
   * @brief Access the element with given index.
   */
  inline const T& operator[](size_type index) const {
    return *(static_cast<const TDerived&>(*this).begin() + index);
  }

  /**
   * @copybrief operator[]()
   */
  inline T& operator[](size_type index) {
    return const_cast<T&>(const_cast<const ContiguousContainerMixin&>(*this)[index]);
  }

  /**
   * @brief Access the first element.
   */
  inline const T& front() const {
    return *(static_cast<const TDerived&>(*this).begin());
  }

  /**
   * @copybrief front()const
   */
  inline T& front() {
    return const_cast<T&>(const_cast<const ContiguousContainerMixin&>(*this).front());
  }

  /**
   * @brief Access the last element.
   */
  inline const T& back() const {
    return *(static_cast<const TDerived&>(*this).end() - 1);
  }

  /**
   * @copybrief back()const
   */
  inline T& back() {
    return const_cast<T&>(const_cast<const ContiguousContainerMixin&>(*this).back());
  }

  /// @group_iterators

  /**
   * @brief Get an iterator to the beginning.
   */
  iterator begin() {
    return const_cast<iterator>(const_cast<const TDerived&>(static_cast<TDerived&>(*this)).begin()); // TODO cleaner?
  }

  /**
   * @copybrief begin()
   */
  const_iterator cbegin() {
    return const_cast<const TDerived&>(static_cast<TDerived&>(*this)).begin(); // TODO cleaner?
  }

  /**
   * @brief Get an iterator to the end.
   */
  iterator end() {
    return const_cast<iterator>(const_cast<const TDerived&>(static_cast<TDerived&>(*this)).end()); // TODO cleaner?
  }

  /**
   * @copybrief end()const
   */
  const_iterator cend() {
    return const_cast<const TDerived&>(static_cast<TDerived&>(*this)).end(); // TODO cleaner?
  }

  /// @group_operations

  /**
   * @brief Check equality.
   */
  virtual bool operator==(const TDerived& rhs) const {
    const auto& lhs = static_cast<const TDerived&>(*this);
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
  }

  /**
   * @brief Check inequality.
   */
  bool operator!=(const TDerived& rhs) const {
    return not(*this == rhs);
  }

  /// @}
};

/**
 * @relatesalso ContiguousContainerMixin
 * @brief Insert a `ContiguousContainerMixin` into an output stream.
 */
template <typename T, typename TDerived>
std::ostream& operator<<(std::ostream& os, const ContiguousContainerMixin<T, TDerived>& container) {
  os << "[";
  if (not container.empty()) {
    const auto size = static_cast<const TDerived&>(container).size();
    std::size_t i = 0;
    os << container[i];
    if (size > 7) {
      for (++i; i < 3; ++i) {
        os << ", " << container[i];
      }
      i = size - 3;
      os << " ... " << container[i];
    }
    for (++i; i < size; ++i) {
      os << ", " << container[i];
    }
  }
  os << "]";
  return os;
}

} // namespace Linx

#endif
