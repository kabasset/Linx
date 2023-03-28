// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXBASE_ALIGNEDBUFFER_H
#define _LINXBASE_ALIGNEDBUFFER_H

#include "LinxBase/Exceptions.h"
#include "LinxBase/TypeUtils.h"

#include <algorithm> // copy_n
#include <cstdint> // uintptr_t
#include <sstream>

namespace Linx {

/**
 * @relates AlignedBuffer
 * @brief Check whether some pointer is aligned as required.
 */
template <typename T>
bool isAligned(const T* ptr, Index as) {
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
Index alignment(const T* ptr) {
  if (not ptr) {
    throw NullPtrError("Null pointer tested for alignment.");
  }
  Index as = 2; // FIXME 1?
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
  AlignmentError(const void* ptr, Index alignment) :
      Exception("Alignment error", toString(ptr) + " is not " + std::to_string(alignment) + " byte-aligned.") {};

  /**
   * @brief Throw if the alignement requirement is not met.
   */
  static void mayThrow(const void* ptr, Index alignment) {
    if (not isAligned(ptr, alignment)) {
      throw AlignmentError(ptr, alignment);
    }
  }
};

/**
 * @ingroup data_classes
 * @brief Data holder with aligned memory.
 * 
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
   * 
   * If `data = nullptr`, the buffer is owning the data,
   * and some aligned memory is allocated.
   * In this case, if `align` is-1 or 0, alignment is made compatible with SIMD instructions.
   * 
   * \snippet LinxDemoConstructors_test.cpp AlignedRaster owns
   * 
   * If the buffer is not owning (`data` is not null),
   * `data` alignment is checked, unless `align` is 0.
   * 
   * \snippet LinxDemoConstructors_test.cpp AlignedRaster shares
   */
  AlignedBuffer(std::size_t size, T* data = nullptr, Index align = 0) :
      m_container(nullptr), m_begin(data), m_end(data + size), m_as(alignAs(data, align)) {
    if (m_begin) {
      AlignmentError::mayThrow(m_begin, m_as);
    } else {
      allocate(size);
    }
  }

  /**
   * @brief Copy constructor.
   */
  AlignedBuffer(const AlignedBuffer& other) :
      AlignedBuffer(other.m_end - other.m_begin, other.owns() ? nullptr : other.m_begin, other.m_as) {
    if (other.owns()) {
      std::copy(other.m_begin, other.m_end, const_cast<std::remove_cv_t<T>*>(m_begin));
      // Safe because if T is const, other is not owning (or should we throw?)
    }
  }

  /**
   * @brief Move constructor.
   */
  AlignedBuffer(AlignedBuffer&& other) : m_container(), m_begin(other.m_begin), m_end(other.m_end), m_as(other.m_as) {
    other.release();
    other.reset();
  }

  /**
   * @brief Copy assignment.
   */
  AlignedBuffer& operator=(const AlignedBuffer& other) {
    if (this != &other) {
      m_as = other.m_as; // Must be set before allocate()
      if (other.owns()) {
        allocate(other.m_end - other.m_begin);
        std::copy(other.m_begin, other.m_end, m_begin);
      } else {
        m_container = other.m_container;
        m_begin = other.m_begin;
        m_end = other.m_end;
      }
    }
    return *this;
  }

  /**
   * @brief Move assignment.
   */
  AlignedBuffer& operator=(AlignedBuffer&& other) {
    if (this != &other) {
      m_container = other.m_container;
      m_begin = other.m_begin;
      m_end = other.m_end;
      m_as = other.m_as;
      other.release();
      other.reset();
    }
    return *this;
  }

  /**
   * @brief Destructor.
   * 
   * Frees memory if owned.
   */
  ~AlignedBuffer() {
    reset();
  }

  /**
   * @brief Get an iterator to the beginning.
   */
  inline const T* begin() const {
    return m_begin;
  }

  /**
   * @brief Get an iterator to the end.
   */
  inline const T* end() const {
    return m_end;
  }

  /**
   * @brief Check whether the data is owned by this object.
   * 
   * Returns false if the data is owned by another object,
   * or if it is empty, e.g. was owned but has been moved.
   */
  bool owns() const {
    return m_container;
  }

  /**
   * @brief Get the required data alignment.
   */
  std::size_t alignmentReq() const { // FIXME overalignment() as in std doc?
    return m_as;
  }

  /**
   * @brief Get the actual data alignment, which may be better than required.
   */
  std::size_t alignment() const {
    return Linx::alignment(m_begin);
  }

  /**
   * @brief Release the memory.
   * 
   * The buffer can still be used, but does not own the data anymore,
   * and thus memory will not be freed when it goes out of scope.
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
   * 
   * Size is set to 0, alignment requirement to 1, and pointers are nullified.
   * If the buffer is owned, memory is freed.
   */
  void reset() {
    if (m_container) {
      std::free(m_container);
      m_container = nullptr;
    }
    m_as = 1;
    m_begin = nullptr;
    m_end = nullptr;
  }

private:
  void allocate(std::size_t size) {
    const auto validSize = ((size + m_as - 1) / m_as) * m_as; // Smallest multiple of m_as >= size
    m_container = std::aligned_alloc(m_as, sizeof(T) * validSize);
    m_begin = reinterpret_cast<T*>(m_container);
    m_end = m_begin + size;
  }

  static std::size_t alignAs(const void* data, Index align) {
    constexpr std::size_t simd = 32; // 64 for AVX512; TODO detect?
    switch (align) {
      case -1:
        return simd;
      case 0:
        return data ? 1 : simd;
      default:
        return static_cast<std::size_t>(align);
    }
  }

protected:
  /**
   * @brief The container if owned, or `nullptr` if shared.
   */
  void* m_container;

  /**
   * @brief The buffer beginning.
   */
  T* m_begin;

  /**
   * @brief The buffer end.
   */
  T* m_end;

  /**
   * @brief The required alignment.
   */
  Index m_as; // FIXME rm?
};

} // namespace Linx

#endif
