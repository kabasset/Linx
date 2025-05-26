// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_MAP_H
#define LINX_DATA_MAP_H

#include "Linx/Base/Types.h"
#include "Linx/Base/mixins/Data.h"
#include "Linx/Base/mixins/Range.h"
#include "Linx/Data/Image.h" // for ExpandPath
#include "Linx/Data/Sequence.h"

#include <Kokkos_Pair.hpp>
#include <concepts>
#include <string>
#include <unordered_map>

namespace Linx {

/**
 * @brief A sequence of positions.
 */
template <int N>
class Path {
public:

  static constexpr int n = N;
  using size_type = std::size_t;
  using value_type = const Position<N>;
  using reference = value_type&;
  using const_reference = const value_type&;

  /**
   * @brief Constructor.
   */
  Path(const std::string& label, std::integral auto size) : m_label(label), m_path(size) {}

  /**
   * @brief Path label.
   */
  KOKKOS_INLINE_FUNCTION std::string label() const
  {
    return m_label;
  }

  /**
   * @brief Rank of the positions.
   */
  KOKKOS_INLINE_FUNCTION int rank() const
  {
    return m_path.begin()->ssize();
  }

  /**
   * @brief Number of positions.
   */
  KOKKOS_INLINE_FUNCTION size_type size() const
  {
    return m_path.size();
  }

  /**
   * @brief Number of positions.
   */
  KOKKOS_INLINE_FUNCTION auto ssize() const
  {
    return static_cast<std::make_signed_t<size_type>>(size());
  }

  /**
   * @brief Access the i-th position.
   */
  KOKKOS_INLINE_FUNCTION reference operator[](auto i) const
  {
    return m_path[i];
  }

  /**
   * @brief Access the i-th position.
   */
  KOKKOS_INLINE_FUNCTION reference operator()(auto i) const
  {
    return m_path[i];
  }

  /**
   * @brief Access the j-th element of the i-th position.
   */
  KOKKOS_INLINE_FUNCTION decltype(auto) operator()(auto i, auto j) const
  {
    return m_path[i][j];
  }

  /**
   * @brief Iterator to the beginning.
   */
  auto begin() const
  {
    return m_path.begin();
  }

  /**
   * @brief Iterator to the end.
   */
  auto end() const
  {
    return m_path.end();
  }

private:

  std::string m_label; ///< The label
  std::vector<Position<N>> m_path; ///< The positions, converted to Image in for_each()
};

template <int N>
KOKKOS_INLINE_FUNCTION decltype(auto) as_readonly(const Path<N>& in)
{
  return in; // FIXME const value_type
}

/**
 * @brief Mapping from positions to values.
 * 
 * As opposed to an image, map is a resizable container, whose memory is always on host.
 * Trying to access an element out of the map domain returns a reference to the default value `out_of_bounds`,
 * such that a map is naturally extrapolated, with using this constant value.
 * 
 * Copy is shallow by default.
 */
template <typename T, int N>
class Map :
    DataMixin<T, CopyArithmeticMixin<T, Map<T, N>>, Map<T, N>>,
    RangeMixin<true, T, Map<T, N>> { // FIXME arithmetic // FIXME GMap?
public:

  using value_type = T;
  using element_type = std::remove_cvref_t<value_type>;
  using reference = value_type&;
  using const_reference = const value_type&;
  using size_type = std::size_t; // FIXME

  /**
   * @brief Constructor.
   * @param label The map label
   * @param out The value returned when out-of-domain elements are accessed
   */
  Map(const std::string& label = "", const T& out = T()) :
      out_of_range(out),
      m_map_view(Kokkos::view_alloc(label, Kokkos::SequentialHostInit)),
      m_map(*m_map_view.data())
  {}

  /**
   * @brief Map label.
   */
  std::string label() const
  {
    return m_map_view.label();
  }

  /**
   * @brief Domain rank.
   * 
   * @warning The rank is undefined if the domain is empty.
   */
  KOKKOS_INLINE_FUNCTION auto rank() const
  {
    return begin()->rank();
  }

  /**
   * @brief Domain size.
   */
  KOKKOS_INLINE_FUNCTION size_type size() const
  {
    return m_map.size();
  }

  /**
   * @brief Domain size.
   */
  KOKKOS_INLINE_FUNCTION auto ssize() const
  {
    return static_cast<std::make_signed_t<size_type>>(size());
  }

  /**
   * @brief Sequence of positions in the map (deep copy).
   */
  auto domain() const
  {
    auto out = Path<N>(compose_label("domain", label()), ssize());
    auto it = out.begin();
    for (auto kv : m_map) {
      for (int i = 0; i < it->ssize(); ++i) {
        (*it)[i] = kv.first[i];
      }
      // it->assign(kv.first.begin());
      ++it;
    }
    return out;
  }

  /**
   * @brief Sequence of values in the map (deep copy).
   */
  auto values() const
  {
    auto out = GPosition<T, -1>(compose_label("values", label()), size()); // FIXME Sequence?
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
  KOKKOS_INLINE_FUNCTION decltype(auto) begin() const
  {
    return m_map.begin();
  }

  /**
   * @brief Position-value pair iterator to the end.
   */
  KOKKOS_INLINE_FUNCTION decltype(auto) end() const
  {
    return m_map.end();
  }

  /**
   * @brief Access the element at given indices.
   */
  KOKKOS_INLINE_FUNCTION reference at(std::integral auto... is) const
  {
    for (auto& pair : m_map) {
      if (pair.first.equal(is...)) {
        return const_cast<reference>(pair.second); // Mutable element
      }
    }
    return const_cast<reference>(out_of_range);
  }

  /**
   * @brief Access the element at given indices.
   */
  KOKKOS_INLINE_FUNCTION reference operator()(std::integral auto... is) const
  {
    return at(is...);
  }

  /**
   * @brief Access the element at given indices if it exists or insert it otherwise.
   */
  reference operator()(std::integral auto... is)
  {
    auto ptr = &at(is...);
    if (ptr == &out_of_range) {
      return m_map.emplace_back(Position<N> {is...}, T()).second;
    }
    return *ptr;
  }

  /**
   * @brief Access the element at given position.
   */
  reference at(const Position<N>& p) const
  {
    for (auto& pair : m_map) {
      if (pair.first == p) {
        return const_cast<reference>(pair.second); // Mutable element
      }
    }
    return const_cast<reference>(out_of_range);
  }

  /**
   * @brief Access the element at given position.
   */
  reference operator[](const Position<N>& p) const
  {
    return at(p);
  }

  /**
   * @brief Access the element at given position if it exists or insert it otherwise.
   */
  reference operator[](const Position<N>& p)
  {
    auto ptr = &(const_cast<const Map&>(*this)[p]);
    if (ptr == &out_of_range) {
      return m_map.emplace_back(+p, T()).second;
    }
    return *ptr;
  }

public:

  value_type out_of_range; ///< Value returned when position is out of range

private:

  using Container = std::vector<Kokkos::pair<Position<N>, T>>;

  Kokkos::View<Container, Kokkos::HostSpace> m_map_view; ///< Shared memory management
  Container& m_map; ///< The position-value pairs
};

template <typename T, int N>
KOKKOS_INLINE_FUNCTION decltype(auto) as_readonly(const Map<T, N>& in)
{
  return in; // FIXME const value_type
}

template <typename T, int N>
KOKKOS_INLINE_FUNCTION decltype(auto) on_host(const Map<T, N>& in)
{
  return in; // `in` cannot be on device
}

/**
 * @brief Get the bounding box of a sequence of positions.
 */
template <int N>
auto box(const Path<N>& in)
{
  auto it = in.begin();
  auto out = Box(*it, *it);
  for (++it; it != in.end(); ++it) {
    const auto& p = *it;
    for (std::size_t i = 0; i < p.size(); ++i) {
      auto& start_i = out.start(i);
      auto& stop_i = out.stop(i);
      auto p_i = p[i];
      start_i = std::min(start_i, p_i);
      stop_i = std::max(stop_i, p_i + 1);
    }
  }
  return out;
}

template <typename TSpace, int N, typename TFunc> // FIXME support -1?
struct ExpandPath {
  ExpandPath(const Path<N>& path, TFunc func) : m_view("Path", path.size(), path.rank()), m_func(LINX_MOVE(func))
  {
    const auto& h = on_host(m_view);
    h.copy_from(path);
    Kokkos::deep_copy(m_view.container(), h.container()); // FIXME as method? in copy_from/to()?
  }

  KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto i) const
  {
    return apply_impl(i, std::make_index_sequence<N>());
  }

  template <std::size_t... Is>
  KOKKOS_INLINE_FUNCTION auto apply_impl(auto i, std::index_sequence<Is...>) const
  {
    return m_func(m_view(i, Is)...);
  }

  Image<int, 2, ImageContainer<int, 2, TSpace>> m_view;
  TFunc m_func;
};

/**
 * @brief Apply a function to each element of the domain.
 * @tparam TSpace The execution space
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, int N, typename TFunc>
void for_each(const std::string& label, Path<N> region, TFunc func)
{
  for_each<TSpace>(
      label,
      Slice(0L, region.ssize()), // FIXME accept different types in Slice
      ExpandPath<TSpace, N, TFunc>(region, func));
}

} // namespace Linx

#endif
