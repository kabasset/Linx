// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_ALIGNEDBUFFER_H
#define _RASTER_ALIGNEDBUFFER_H

#include "RasterTypes/Exceptions.h"

#include <fftw3.h> // fftw_malloc, fftw_alignment_of, fftw_free

namespace Cnes {

/// @cond
namespace Internal {

template <typename T>
T* alignedAlloc(std::size_t size) {
  return (T*)fftw_malloc(sizeof(T) * size);
}

template <typename T>
bool isAligned(T* data) {
  return fftw_alignment_of(reinterpret_cast<double*>(data)) == 0;
}

template <typename T>
bool isAligned(const T* data) {
  return isAligned(const_cast<T*>(data)); // FIXME safe?
}

template <typename T>
void alignedFree(T* data) {
  fftw_free(data);
}

template <typename T>
void alignedFree(const T*) {}

} // namespace Internal
/// @endcond

/**
 * @brief Data holder with SIMD-friendly memory.
 * @details
 * The data pointer is guaranteed to be aligned in memory
 * according to the available SIMD technology (generally, 16-byte aligned).
 * Data can be either owned by the object, or shared and owned by another object.
 * In the latter case, alignment is tested at construction.
 */
template <typename T>
struct AlignedBuffer {

public:
  /**
   * @brief The concrete container type.
   */
  using Container = T*;

  /**
   * @brief Constructor.
   * @details
   * Allocate some aligned memory if `data = nullptr`.
   * Check for alignment of `data` otherwise.
   */
  AlignedBuffer(std::size_t size, T* data = nullptr) :
      m_shared(data), m_size(size), m_container(data ? data : Internal::alignedAlloc<T>(m_size)) {
    if (m_shared && not Internal::isAligned(m_container)) {
      throw Exception("Provided data pointer is not correctly aligned."); // FIXME
    }
  }

  /**
   * @brief Non-copyable.
   */
  AlignedBuffer(const AlignedBuffer&) = delete;

  /**
   * @brief Movable.
   */
  AlignedBuffer(AlignedBuffer&& other) :
      m_shared(other.m_shared), m_size(other.m_size), m_container(other.m_container) {
    other.m_shared = false;
    other.m_size = 0;
    other.m_container = nullptr;
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
      m_shared = other.m_shared;
      m_size = other.m_size;
      m_container = other.m_container;
      other.m_shared = false;
      other.m_size = 0;
      other.m_container = nullptr;
    }
    return *this;
  }

  /**
   * @brief Destructor.
   * @details
   * Frees memory if needed.
   */
  ~AlignedBuffer() {
    if (owns()) {
      Internal::alignedFree(m_container);
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
    return m_container;
  }

  /**
   * @brief Check whether the data is owned by this object.
   * @details
   * Returns false if the data is owned by another object,
   * or if it is empty, e.g. was owned but has been moved.
   */
  bool owns() const {
    return m_container && not m_shared;
  }

protected:
  /**
   * @brief Is the data shared?
   */
  bool m_shared;

  /**
   * @brief The data size.
   */
  std::size_t m_size;

  /**
   * @brief The data pointer.
   */
  T* m_container;
};

} // namespace Cnes

#endif // _RASTER_ALIGNEDBUFFER_H
