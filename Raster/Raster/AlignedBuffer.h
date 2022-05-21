// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_ALIGNEDBUFFER_H
#define _RASTER_ALIGNEDBUFFER_H

#include "RasterTypes/Exceptions.h"

#include <cstdint> // uintptr_t
#include <utility> // pair

namespace Cnes {

/// @cond
namespace Internal {

template <typename T>
bool isAligned(const T* data, std::size_t as) {
  if (not data) {
    throw Exception("Null pointer tested for alignment.");
  }
  if (as == 1) {
    return true;
  }
  return std::uintptr_t(data) % as == 0;
}

template <typename T>
std::uintptr_t alignment(const T* data) {
  if (not data) {
    throw Exception("Null pointer tested for alignment.");
  }
  std::size_t as = 2;
  while (std::uintptr_t(data) % as == 0) {
    as <<= 1;
  }
  return as >> 1;
}

template <typename T>
std::pair<void*, T*> alignedAlloc(std::size_t size, T* data, std::size_t as) {
  if (data) {
    if (isAligned(data, as)) {
      return {nullptr, data};
    }
    throw Exception("Provided data pointer is not correctly aligned.");
  }
  void* p = std::malloc(sizeof(T) * size + as - 1);
  return {p, reinterpret_cast<T*>((std::uintptr_t(p) + (as - 1)) & ~(as - 1))};
}

template <typename T>
void alignedFree(std::pair<void*, T*>& container) {
  if (container.first) {
    std::free(container.first);
  }
}

template <typename T>
void alignedFree(const std::pair<void*, const T*>&) {}

} // namespace Internal
/// @endcond

/**
 * @brief Data holder with SIMD-friendly memory.
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
  using Container = std::pair<void*, T*>;

  /**
   * @brief Constructor.
   * @details
   * Allocate some aligned memory if `data = nullptr`.
   * Check for alignment of `data` otherwise.
   */
  AlignedBuffer(std::size_t size, T* data = nullptr, std::size_t align = 16) :
      m_size(size), m_container(Internal::alignedAlloc(size, data, align)) {}

  /**
   * @brief Non-copyable.
   */
  AlignedBuffer(const AlignedBuffer&) = delete;

  /**
   * @brief Movable.
   */
  AlignedBuffer(AlignedBuffer&& other) : m_size(other.m_size), m_container(other.m_container) {
    other.m_size = 0;
    other.m_container.first = nullptr;
    other.m_container.second = nullptr;
  }

  /**
   * @brief Non-copyable.
   */
  AlignedBuffer& operator=(const AlignedBuffer&) = delete;

  /**
   * @brief Movable.
   */
  AlignedBuffer& operator=(AlignedBuffer&& other) {
    if (this != &other) {
      m_size = other.m_size;
      m_container = other.m_container;
      other.m_size = 0;
      other.m_container = {nullptr, nullptr};
    }
    return *this;
  }

  /**
   * @brief Destructor.
   * @details
   * Frees memory if needed.
   */
  ~AlignedBuffer() {
    Internal::alignedFree(m_container);
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
    return m_container.second;
  }

  /**
   * @brief Check whether the data is owned by this object.
   * @details
   * Returns false if the data is owned by another object,
   * or if it is empty, e.g. was owned but has been moved.
   */
  bool owns() const {
    return m_container.first;
  }

  /**
   * @brief Get the actual data alignment, which may be better than required.
   */
  std::uintptr_t alignment() const {
    return Internal::alignment(m_container.second);
  }

protected:
  /**
   * @brief The data size.
   */
  std::size_t m_size;

  /**
   * @brief The unaligned container.
   */
  std::pair<void*, T*> m_container;
};

} // namespace Cnes

#endif // _RASTER_ALIGNEDBUFFER_H
