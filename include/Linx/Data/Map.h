// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_MAP_H
#define LINX_DATA_MAP_H

#include "Linx/Base/Types.h"
#include "Linx/Base/mixins/Data.h"
#include "Linx/Base/mixins/Range.h"
#include "Linx/Data/Sequence.h"

#include <concepts>
#include <string>
#include <unordered_map>

namespace Linx {

/**
 * @brief Mapping from positions to values.
 */
template <typename T, int N>
class Map : DataMixin<T, void, Map<T, N>>, RangeMixin<true, T, Map<T, N>> { // FIXME arithmetic
public:

  using value_type = T;
  using element_type = std::remove_cvref_t<value_type>;
  using reference = value_type&;
  using const_reference = const value_type&;
  using size_type = std::size_t; // FIXME

  /**
   * @brief Constructor.
   */
  Map(const std::string& label = "") : m_label(label), m_map() {}

  /**
   * @brief The map label.
   */
  KOKKOS_INLINE_FUNCTION const std::string& label() const
  {
    return m_label;
  }

  /**
   * @brief The domain size.
   */
  KOKKOS_INLINE_FUNCTION size_type size() const
  {
    return m_map.size();
  }

  /**
   * @brief The domain size.
   */
  KOKKOS_INLINE_FUNCTION auto ssize() const
  {
    return static_cast<std::make_signed_t<size_type>>(size());
  }

  /**
   * @brief Get the sequence of positions in the map (shallow copy).
   */
  auto domain() const
  {
    auto out = Sequence<Position<N>, -1>(compose_label("domain", m_label), size());
    auto it = out.begin();
    for (auto kv : m_map) {
      *it = kv.first;
      ++it;
    }
    return out;
  }

  /**
   * @brief Get the sequence of values in the map.
   */
  auto values() const
  {
    auto out = Sequence<T, -1>(compose_label("values", m_label), size());
    auto it = out.begin();
    for (auto kv : m_map) {
      *it = kv.second;
      ++it;
    }
    return out;
  }

  /**
   * @brief Position-value pair iterator to the beginnig.
   */
  KOKKOS_INLINE_FUNCTION auto begin() const
  {
    return m_map.begin();
  }

  /**
   * @brief Position-value pair iterator to the end.
   */
  KOKKOS_INLINE_FUNCTION auto end() const
  {
    return m_map.end();
  }

  /**
   * @brief Access the element at given position.
   */
  reference operator()(std::integral auto... is) const // FIXME KOKKOS_INLINE_FUNCTION
  {
    return operator[]({is...}); // FIXME no assignment, implement loop
  }

  /**
   * @brief Access the element at given position.
   */
  KOKKOS_INLINE_FUNCTION reference operator[](const Position<N>& p) const
  {
    for (auto& pair : m_map) {
      if (pair.first == p) {
        return const_cast<reference>(pair.second); // Mutable element
      }
    }
    throw std::out_of_range(p.label());
  }

  /**
   * @brief Access the element at given position if it exists or insert it.
   */
  reference operator[](const Position<N>& p)
  {
    try {
      return const_cast<const Map&>(*this)[p];
    } catch (std::out_of_range&) {
      return m_map.emplace_back(+p, T()).second;
    }
  }

private:

  std::string m_label; ///< The label
  std::vector<std::pair<Position<N>, T>> m_map; ///< The position-value pairs
};

/**
 * @brief Perform a shallow copy of a sequence, as a readonly sequence.
 * 
 * If the input sequence is aleady readonly, then this is a no-op.
 */
template <typename T, int N>
KOKKOS_INLINE_FUNCTION decltype(auto) as_readonly(const Map<T, N>& in)
{
  return in; // FIXME
}

} // namespace Linx

#endif
