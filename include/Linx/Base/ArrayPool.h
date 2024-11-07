// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_ARRAYPOOL_H
#define LINX_BASE_ARRAYPOOL_H

#ifndef KOKKOS_IMPL_PUBLIC_INCLUDE
#define KOKKOS_IMPL_PUBLIC_INCLUDE
#include "Kokkos_UniqueToken.hpp"
#undef KOKKOS_IMPL_PUBLIC_INCLUDE
#else
#include "Kokkos_UniqueToken.hpp"
#endif

#include <cstddef> // size_t
#include <type_traits> // remove_cvref

namespace Linx {

/**
 * @brief A pool of arrays for pre-allocating thread-wise memory.
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
class ArrayPool {
  friend class Array;

private:

  using execution_space = TSpace; ///< The execution space
  using device_type = typename execution_space::device_type; ///< The device type

public:

  /**
   * @brief A non-owning array, which is returned by the pool.
   */
  class Array {
  public:

    using value_type = T; ///< The raw value type
    using element_type = std::remove_cvref_t<T>; ///< The decayed value type
    using pointer = value_type*; ///< The pointer type
    using reference = value_type&; ///< The reference type

    /**
     * @brief Constructor (acquires memory).
     */
    KOKKOS_INLINE_FUNCTION Array(const ArrayPool& pool) :
        m_pool(pool),
        m_index(m_pool.acquire()),
        m_data(&m_pool.m_memory(m_index, 0)),
        m_size(m_pool.m_memory.extent(1))
    {}

    /**
     * @brief Destructor (releases memory).
     */
    KOKKOS_INLINE_FUNCTION ~Array()
    {
      m_pool.release(m_index);
    }

    /**
     * @brief Array size.
     */
    KOKKOS_INLINE_FUNCTION std::size_t size() const
    {
      return m_size;
    }

    /**
     * @brief Pointer to the data. 
     */
    KOKKOS_INLINE_FUNCTION pointer data()
    {
      return m_data;
    }

    /**
     * @brief Access the element at given index.
     */
    KOKKOS_INLINE_FUNCTION reference operator[](Index i)
    {
      return m_data[i];
    }

  private:

    const ArrayPool& m_pool; ///< Parent pool
    Index m_index; ///< In-pool index
    T* m_data; ///< Data pointer
    std::size_t m_size; ///< Array size
  };

  /**
   * @brief Constructor.
   * @param size The size of each array
   */
  ArrayPool(std::size_t size) : m_tokens(), m_memory("memory", m_tokens.size(), size) {}

  /**
   * @brief Get one of the arrays. 
   */
  KOKKOS_INLINE_FUNCTION Array array() const
  {
    return Array(*this);
  }

private:

  /**
   * @brief Acquire an array and get its index.
   */
  KOKKOS_INLINE_FUNCTION auto acquire() const
  {
    return m_tokens.acquire();
  }

  /**
   * @brief Release an array.
   */
  KOKKOS_INLINE_FUNCTION void release(auto i) const
  {
    m_tokens.release(i);
  }

private:

  Kokkos::Experimental::UniqueToken<execution_space> m_tokens; ///< The thread tokens
  Kokkos::View<T**, device_type> m_memory; ///< The actual memory
};

} // namespace Linx

#endif
