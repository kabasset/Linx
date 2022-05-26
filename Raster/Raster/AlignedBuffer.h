// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_ALIGNEDBUFFER_H
#define _RASTER_ALIGNEDBUFFER_H

#include "Raster/DataUtils.h"
#include "RasterTypes/Exceptions.h"

#include <cstdint> // uintptr_t
#include <sstream>

namespace Cnes {

/**
 * @brief Check whether some pointer is aligned as required.
 */
template <typename T>
bool isAligned(const T* ptr, std::size_t as) {
  if (not ptr) {
    throw Exception("Null pointer tested for alignment.");
  }
  if (as == 1) {
    return true;
  }
  return std::uintptr_t(ptr) % as == 0;
}

/**
 * @brief Get the highest power of two some pointer is aligned as.
 */
template <typename T>
std::size_t alignment(const T* ptr) {
  if (not ptr) {
    throw Exception("Null pointer tested for alignment.");
  }
  std::size_t as = 2;
  while (std::uintptr_t(ptr) % as == 0) {
    as <<= 1;
  }
  return as >> 1;
}

/**
 * @brief Convert a pointer address to string.
 */
inline std::string toString(const void* ptr) {
  std::stringstream ss;
  ss << ptr;
  return ss.str();
}

/**
 * @brief Exception thrown when an alignment requirement is not met.
 */
struct AlignmentError : Exception {
  /**
   * @brief Constructor.
   */
  AlignmentError(const void* ptr, std::size_t alignment) :
      Exception(toString(ptr) + " is not " + std::to_string(alignment) + " byte-aligned.") {};

  /**
   * @brief Throw if the alignement requirement is not met.
   */
  static void mayThrow(const void* ptr, std::size_t alignment) {
    if (not isAligned(ptr, alignment)) {
      throw AlignmentError(ptr, alignment);
    }
  }
};

/**
 * @brief Data holder with aligned memory.
 * @details
 * The data pointer is guaranteed to be aligned in memory
 * according to the alignment requirement (might be better).
 * Data can be either owned by the object, or shared and owned by another object.
 * In the latter case, alignment is tested against requirement at construction.
 */
template <typename T>
struct AlignedBuffer {

public:
  /**
   * @brief The concrete container type.
   */
  using Container = void*;

  /**
   * @brief Constructor.
   * @param size The number of elements
   * @param data The data pointer if it pre-exists, or `nullptr` otherwise
   * @param align The alignment in bytes, or 0 for default (SIMD-compatible)
   * @details
   * Allocate some aligned memory if `data = nullptr`.
   * Check for alignment of `data` otherwise.
   */
  AlignedBuffer(std::size_t size, T* data = nullptr, std::size_t align = 0) :
      m_size(size), m_container(nullptr), m_data(data) {
    const auto as = align ? align : 64; // 64 for AVX512; TODO detect?
    if (m_data) {
      AlignmentError::mayThrow(m_data, as);
    } else {
      m_container = std::malloc(sizeof(T) * m_size + as - 1);
      m_data = reinterpret_cast<T*>((std::uintptr_t(m_container) + (as - 1)) & ~(as - 1));
    }
  }

  CNES_NON_COPYABLE(AlignedBuffer)

  /**
   * @brief Move constructor.
   */
  AlignedBuffer(AlignedBuffer&& other) : m_size(other.m_size), m_container(other.m_container), m_data(other.m_data) {
    other.m_size = 0;
    other.m_container = nullptr;
    other.m_data = nullptr;
  }

  /**
   * @brief Move assignment.
   */
  AlignedBuffer& operator=(AlignedBuffer&& other) {
    if (this != &other) {
      m_size = other.m_size;
      m_container = other.m_container;
      m_data = other.m_data;
      other.m_size = 0;
      other.m_container = nullptr;
      other.m_data = nullptr;
    }
    return *this;
  }

  /**
   * @brief Destructor.
   * @details
   * Frees memory if owned.
   */
  ~AlignedBuffer() {
    if (m_container) {
      std::free(m_container);
    }
  }

  /**
     * @brief Get the data size.
     */
  std::size_t size() const {
    return m_size;
  }

  /**
     * @brief Get the data pointer.
     */
  const T* data() const {
    return m_data;
  }

  /**
     * @brief Check whether the data is owned by this object.
     * @details
     * Returns false if the data is owned by another object,
     * or if it is empty, e.g. was owned but has been moved.
     */
  bool owns() const {
    return m_container;
  }

  /**
     * @brief Get the actual data alignment, which may be better than required.
     */
  std::size_t alignment() const {
    return Cnes::alignment(m_data);
  }

protected:
  /**
     * @brief The data size.
     */
  std::size_t m_size;

  /**
     * @brief The unaligned container.
     */
  Container m_container;

  /**
     * @brief The aligned data.
     */
  T* m_data;
};

} // namespace Cnes

#endif // _RASTER_ALIGNEDBUFFER_H
