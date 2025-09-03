// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_BOX_H
#define LINX_DATA_BOX_H

#include "Linx/Base/Containers.h"
#include "Linx/Base/Exceptions.h"
#include "Linx/Base/Functional.h"
#include "Linx/Base/Packs.h"
#include "Linx/Base/Slice.h"
#include "Linx/Base/Types.h"
#include "Linx/Base/concepts/Array.h"
#include "Linx/Data/Vector.h"

#include <Kokkos_Core.hpp>
#include <concepts>
#include <string>

namespace Linx {

namespace Impl {

template <typename T, int N>
struct VectorTraits {
  using data_type = T[N];
};

template <typename T>
struct VectorTraits<T, -1> {
  using data_type = T*;
};

template <typename T>
struct VectorTraits<T, 0> {
  using data_type = std::integer_sequence<T>;
};

template <typename T0, typename T1>
using CommonVector = Vector<typename VectorTraits<
    std::common_type_t<typename T0::value_type, typename T1::value_type>,
    Impl::common_size<T0::n, T1::n>()>::data_type>;

} // namespace Impl

/**
 * @ingroup regions
 * @brief An ND axis-aligned bounding box, defined by its start (inclusive) and stop (exclusive) bounds.
 * @tparam TStart The start vector coefficients specification
 * @tparam TStop The stop vector coefficients specification
 * 
 * If the start and stop vectors are not statically empty, then they must have the same size.
 * 
 * If start and stop vectors have integral coefficients,
 * the box can be iterated with `for_each()` and `kokkos_reduce()`, and patches can be created from the box.
 * 
 * @see `Patch`
 */
template <typename TStart, typename TStop>
class Box {
public:

  using start_type = Vector<TStart>; ///< The start vector type
  using stop_type = Vector<TStop>; ///< The stop vector type

  using size_type = std::size_t; ///< The size type
  using value_type = Impl::CommonVector<start_type, stop_type>; ///< The position type
  using index_type = std::remove_cvref_t<typename value_type::value_type>; ///< The coefficient type

  static constexpr int n = value_type::n; ///< The dimension parameter
  static constexpr bool static_rank_flag = value_type::static_size_flag; ///< Static rank flag
  static constexpr bool static_flag = start_type::static_flag && stop_type::static_flag; ///< Static bounds flag
  static constexpr bool static_start_at_origin_flag = start_type::static_zero_flag; ///< Start is origin flag

  /**
   * @brief Default constructor.
   * 
   */
  constexpr Box() : m_start {}, m_stop {} {}

  /**
   * @brief Constructor.
   */
  constexpr Box(const stop_type& stop) : m_start {}, m_stop(stop) {}

  /**
   * @brief Constructor.
   */
  constexpr Box(const start_type& start, const stop_type& stop) : m_start(start), m_stop(stop) {}

  /**
   * @brief The box rank.
   */
  KOKKOS_INLINE_FUNCTION constexpr size_type rank() const
  {
    return Impl::common_size(m_start.size(), m_stop.size());
  }

  /**
   * @brief The box shape.
   */
  constexpr value_type shape() const
  {
    return m_stop - m_start;
  }

  /**
   * @brief The start bound, inclusive.
   */
  KOKKOS_INLINE_FUNCTION constexpr const start_type& start() const
  {
    return m_start;
  }

  /**
   * @brief The stop bound, exclusive.
   */
  KOKKOS_INLINE_FUNCTION constexpr const stop_type& stop() const
  {
    return m_stop;
  }

  /**
   * @brief The finish bound, inclusive.
   */
  KOKKOS_INLINE_FUNCTION constexpr auto finish() const
  {
    return m_stop - 1;
  }

  /**
   * @brief The start bound along given axis.
   */
  KOKKOS_INLINE_FUNCTION constexpr auto start(std::integral auto i) const
  {
    return m_start[i];
  }

  /**
   * @copybrief start()
   */
  KOKKOS_INLINE_FUNCTION constexpr decltype(auto) start(std::integral auto i)
  {
    return m_start[i];
  }

  /**
   * @brief The stop bound along given axis.
   */
  KOKKOS_INLINE_FUNCTION constexpr auto stop(std::integral auto i) const
  {
    return m_stop[i];
  }

  /**
   * @copybrief stop()
   */
  KOKKOS_INLINE_FUNCTION constexpr decltype(auto) stop(std::integral auto i)
  {
    return m_stop[i];
  }

  /**
   * @brief The extent along given axis.
   */
  KOKKOS_INLINE_FUNCTION constexpr auto extent(std::integral auto i) const
  {
    return m_stop[i] - m_start[i];
  }

  /**
   * @brief Product of the extents, may be negative.
   */
  KOKKOS_INLINE_FUNCTION constexpr index_type volume() const // FIXME rename as volume
  {
    index_type out = 1;
    for (size_type i = 0; i < rank(); ++i) {
      out *= extent(i);
    }
    return out;
  }

  /**
   * @brief Unsigned size, only valid for integral coefficients.
   */
  KOKKOS_INLINE_FUNCTION constexpr size_type size() const
  {
    auto v = volume();
    return v <= 0 ? size_type(0) : static_cast<size_type>(v);
  }

  /**
   * @brief Check whether two boxes are equal.
   */
  constexpr bool operator==(const Specialization<Linx::Box> auto& other) const
  {
    return m_start == other.start() && m_stop == other.stop();
  }

  /**
   * @brief Check whether two boxes are different.
   */
  constexpr bool operator!=(const Specialization<Linx::Box> auto& other) const
  {
    return not(*this == other);
  }

  /**
   * @brief Check whether a position lies inside the box.
   */
  bool contains(const auto& position) const
  { // TODO Subscriptable<size_t>? Indexed?
    SizeMismatch::may_throw("position", rank(), position);
    for (std::size_t i = 0; i < rank(); ++i) {
      if (position[i] < m_start[i] || position[i] > m_stop[i]) {
        return false;
      }
    }
    return true;
  }

  /**
   * @copydoc contains()
   */
  bool contains(std::integral auto... is) const // TODO LINX_UFUNC
  {
    return contains(std::array {is...}); // TODO no instantiation
  }

private:

  start_type m_start; ///< The start bound
  stop_type m_stop; ///< The stop bound
};

} // namespace Linx

#include "Linx/Data/Box/BoxIterator.h"
#include "Linx/Data/Box/BoxUnion.h"
#include "Linx/Data/Box/arithmetics.h"
#include "Linx/Data/Box/creation.h"
#include "Linx/Data/Box/funcs.h"
#include "Linx/Data/Box/types.h"

#endif
