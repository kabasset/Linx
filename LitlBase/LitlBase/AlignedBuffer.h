// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLBASE_ALIGNEDBUFFER_H
#define _LITLBASE_ALIGNEDBUFFER_H

#include "LitlBase/Exceptions.h"
#include "LitlBase/TypeUtils.h"

#include <algorithm> // copy_n
#include <cstdint> // uintptr_t
#include <sstream>

namespace Litl {

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
  std::size_t as = 2; // FIXME 1?
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
   * \snippet LitlDemoConstructors_test.cpp AlignedRaster owns
   * 
   * If the buffer is not owning (`data` is not null),
   * `data` alignment is checked, unless `align` is 0.
   * 
   * \snippet LitlDemoConstructors_test.cpp AlignedRaster shares
   */
  AlignedBuffer(std::size_t size, T* data = nullptr, std::size_t align = 0) :
      m_size(size), m_as(alignAs(data, align)), m_container(nullptr), m_data(data) {
    if (m_data) {
      AlignmentError::mayThrow(m_data, m_as);
    } else {
      allocate();
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
        allocate();
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
   * 
   * Frees memory if owned.
   */
  ~AlignedBuffer() {
    reset();
  }

  /**
   * @brief Get the data size.
   */
  inline std::size_t size() const {
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
  std::size_t alignmentReq() const {
    return m_as;
  }

  /**
   * @brief Get the actual data alignment, which may be better than required.
   */
  std::size_t alignment() const {
    return Litl::alignment(m_data);
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
    m_size = 0;
    m_as = 1;
    m_data = nullptr;
  }

private:
  void allocate() {
    const auto validSize = ((m_size + m_as - 1) / m_as) * m_as; // Smallest multiple of m_as >= m_size
    m_container = std::aligned_alloc(m_as, sizeof(T) * validSize);
    m_data = reinterpret_cast<T*>(m_container);
  }

  static std::size_t alignAs(const void* data, std::size_t align) {
    constexpr std::size_t simd = 32; // 64 for AVX512; TODO detect?
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
   * @brief The container if owned, or `nullptr` if shared.
   */
  void* m_container;

  /**
   * @brief The aligned data.
   */
  T* m_data;
};

} // namespace Litl

#endif
