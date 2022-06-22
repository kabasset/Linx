// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLCONTAINER_ALIGNEDBUFFER_H
#define _LITLCONTAINER_ALIGNEDBUFFER_H

#include "RasterTypes/Exceptions.h"
#include "RasterTypes/TypeUtils.h"

#include <algorithm> // copy_n
#include <cstdint> // uintptr_t
#include <sstream>

namespace Cnes {

/**
 * @relates AlignedBuffer
 * @brief Check whether some pointer is aligned as required.
 */
template <typename T>
bool isAligned(const T* ptr, std::size_t as) {
  if (not ptr) {
    throw NullPtrError("Null pointer tested for alignment.");
  }
  if (as == 1) {
    return true;
  }
  return std::uintptr_t(ptr) % as == 0;
}

/**
 * @relates AlignedBuffer
 * @brief Get the highest power of two some pointer is aligned as.
 */
template <typename T>
std::size_t alignment(const T* ptr) {
  if (not ptr) {
    throw NullPtrError("Null pointer tested for alignment.");
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
 * @ingroup exceptions
 * @brief Exception thrown when an alignment requirement is not met.
 */
struct AlignmentError : Exception {
  /**
   * @brief Constructor.
   */
  AlignmentError(const void* ptr, std::size_t alignment) :
      Exception("Alignment error", toString(ptr) + " is not " + std::to_string(alignment) + " byte-aligned.") {};

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
 * @ingroup data_classes
 * @brief Data holder with aligned memory.
 * @details
 * The data pointer is guaranteed to be aligned in memory
 * according to the alignment requirement (might be better).
 * 
 * Data can be either owned by the object, or shared and owned by another object.
 * In the latter case, alignment is tested against requirement at construction.
 * Owning buffers are not copyable, only movable.
 * 
 * Internally, owning buffers rely on a larger memory allocation
 * which guarantees adequate alignment.
 * This larger memory is freed by the destructor,
 * unless the said buffer is `released()`,
 * in which case the user is responsible for freeing it.
 */
template <typename T>
struct AlignedBuffer {

public:
  /**
   * @brief Constructor.
   * @param size The number of elements
   * @param data The data pointer if it pre-exists, or `nullptr` otherwise
   * @param align The alignment requirement in bytes, or 0 or -1 (see below)
   * @details
   * If `data = nullptr`, the buffer is owning the data,
   * and some aligned memory is allocated.
   * In this case, if `align` is-1 or 0, alignment is made compatible with SIMD instructions.
   * 
   * \snippet RasterDemoConstructors_test.cpp AlignedRaster owns
   * 
   * If the buffer is not owning (`data` is not null),
   * `data` alignment is checked, unless `align` is 0.
   * 
   * \snippet RasterDemoConstructors_test.cpp AlignedRaster shares
   */
  AlignedBuffer(std::size_t size, T* data = nullptr, std::size_t align = 0) :
      m_size(size), m_as(alignAs(data, align)), m_container(nullptr), m_data(data) {
    if (m_data) {
      AlignmentError::mayThrow(m_data, m_as);
    } else {
      m_container = std::malloc(sizeof(T) * m_size + m_as - 1);
      m_data = reinterpret_cast<T*>((std::uintptr_t(m_container) + (m_as - 1)) & ~(m_as - 1));
    }
  }

  /**
   * @brief Copy constructor.
   */
  AlignedBuffer(const AlignedBuffer& other) :
      AlignedBuffer(other.m_size, other.owns() ? nullptr : other.m_data, other.m_as) {
    if (other.owns()) {
      std::copy_n(other.m_data, m_size, const_cast<std::remove_cv_t<T>*>(m_data));
      // Safe because if T is const, other is not owning (or should we throw?)
    }
  }

  /**
   * @brief Move constructor.
   */
  AlignedBuffer(AlignedBuffer&& other) : m_size(other.m_size), m_as(other.m_as), m_container(), m_data(other.m_data) {
    other.release();
    other.reset();
  }

  /**
   * @brief Copy assignment.
   */
  AlignedBuffer& operator=(const AlignedBuffer& other) {
    if (this != &other) {
      m_size = other.m_size;
      m_as = other.m_as;
      if (other.owns()) {
        m_container = std::malloc(sizeof(T) * m_size + m_as - 1);
        m_data = reinterpret_cast<T*>((std::uintptr_t(m_container) + (m_as - 1)) & ~(m_as - 1));
        std::copy_n(other.m_data, m_size, m_data);
      } else {
        m_container = other.m_container;
        m_data = other.m_data;
      }
    }
    return *this;
  }

  /**
   * @brief Move assignment.
   */
  AlignedBuffer& operator=(AlignedBuffer&& other) {
    if (this != &other) {
      m_size = other.m_size;
      m_as = other.m_as;
      m_container = other.m_container;
      m_data = other.m_data;
      other.release();
      other.reset();
    }
    return *this;
  }

  /**
   * @brief Destructor.
   * @details
   * Frees memory if owned.
   */
  ~AlignedBuffer() {
    reset();
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
  inline const T* data() const {
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
   * @brief Get the required data alignment.
   */
  std::size_t alignmentReq() const {
    return m_as;
  }

  /**
   * @brief Get the actual data alignment, which may be better than required.
   */
  std::size_t alignment() const {
    return Cnes::alignment(m_data);
  }

  /**
   * @brief Release the memory.
   * @details
   * The buffer can still be used, but does not own the data anymore,
   * and thus memory will not be freed when it goes out of scope.
   * The method returns the pointer to the unaligned memory,
   * i.e. the one which must be freed with `std::free()`.
   * 
   * Aligned memory address is still accessible as `data()`.
   */
  void* release() {
    void* out = m_container;
    m_container = nullptr;
    return out;
  }

  /**
   * @brief Reset the buffer.
   * @details
   * If the buffer is owner, memory is freed.
   * Size is set to 0, alignment requirement to 1, and pointers are nullified.
   */
  void reset() {
    if (m_container) {
      std::free(m_container);
      m_container = nullptr;
    }
    m_size = 0;
    m_as = 1;
    m_data = nullptr;
  }

private:
  static std::size_t alignAs(const void* data, std::size_t align) {
    constexpr std::size_t simd = 64; // 64 for AVX512; TODO detect?
    switch (align) {
      case -1:
        return simd;
      case 0:
        return data ? 1 : simd;
      default:
        return align;
    }
  }

protected:
  /**
   * @brief The data size.
   */
  std::size_t m_size;

  /**
   * @brief The required alignment.
   */
  std::size_t m_as;

  /**
   * @brief The unaligned container.
   */
  void* m_container;

  /**
   * @brief The aligned data.
   */
  T* m_data;
};

} // namespace Cnes

#endif
