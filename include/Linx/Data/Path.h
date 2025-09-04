// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_PATH_H
#define LINX_DATA_PATH_H

#include "Linx/Base/Types.h"
#include "Linx/Base/mixins/Data.h"
#include "Linx/Base/mixins/Range.h"
#include "Linx/Data/Image.h" // for ExpandPath
#include "Linx/Data/Patch.h" // for select()
#include "Linx/Data/Sequence.h"

#include <Kokkos_Pair.hpp>
#include <concepts>
#include <string>
#include <unordered_map>

namespace Linx {

/**
 * @ingroup regions
 * @brief A resizable sequence of positions.
 */
template <int N>
class Path { // FIXME GPath<T, N>? Vector<Position<N>>?
public:

  static constexpr int n = N;
  using size_type = Position<N>::value_type;
  using value_type = const Position<N>;
  using element_type = std::remove_cvref_t<value_type>;
  using reference = value_type&;
  using const_reference = const value_type&;

  /**
   * @brief Constructor.
   * @param label The path label
   * @param size The number of positions
   * @param capacity The preallocated number of position
   */
  explicit Path(const std::string& label, std::size_t capacity) : m_positions(label, capacity, n), m_size("Path size")
  {}

  /**
   * @brief Path label.
   */
  decltype(auto) label() const
  {
    return m_positions.label();
  }

  /**
   * @brief Rank of the positions.
   */
  KOKKOS_INLINE_FUNCTION int rank() const
  {
    return m_positions.extent(1);
  }

  /**
   * @brief Number of positions.
   */
  KOKKOS_INLINE_FUNCTION size_type size() const
  {
    return m_size();
  }

  /**
   * @brief Number of positions.
   */
  KOKKOS_INLINE_FUNCTION auto ssize() const
  {
    return static_cast<std::make_signed_t<size_type>>(size());
  }

  /**
   * @brief Maximum number of positions.
   */
  KOKKOS_INLINE_FUNCTION size_type capacity() const
  {
    return m_positions.extent(0);
  }

  /**
   * @brief Access the i-th position.
   */
  KOKKOS_INLINE_FUNCTION decltype(auto) operator[](std::integral auto i) const
  {
    return m_positions[Slice(int(i))()];
  }

  /**
   * @brief Access the i-th position.
   */
  KOKKOS_INLINE_FUNCTION decltype(auto) operator()(std::integral auto i) const
  {
    return m_positions[Slice(int(i))()];
  }

  /**
   * @brief Access the j-th element of the i-th position.
   */
  KOKKOS_INLINE_FUNCTION size_type& operator()(std::integral auto i, std::integral auto j) const
  {
    return m_positions(i, j);
  }

  /**
   * @brief Append a position.
   */
  KOKKOS_INLINE_FUNCTION void push_back(std::integral auto... position) const
  {
    push_back_impl(forward_as_tuple(position...), std::make_index_sequence<sizeof...(position)>());
  }

private:

  template <std::size_t... Is>
  KOKKOS_INLINE_FUNCTION void push_back_impl(const auto& position, std::index_sequence<Is...>) const
  {
    int index = Kokkos::atomic_fetch_add(&m_size(), 1);
    ((m_positions(index, Is) = get<Is>(position)), ...);
  }

  Image<size_type, 2, ImageView<size_type, 2, Kokkos::HostSpace>> m_positions; ///< The positions
  Kokkos::View<int, Kokkos::HostSpace> m_size; ///< The size
};

/**
 * @relatesalso Path
 */
template <int N>
KOKKOS_INLINE_FUNCTION decltype(auto) as_const(const Path<N>& in)
{
  return in; // FIXME const value_type
}

/**
 * @relatesalso Path
 * @brief Get the bounding box of a sequence of positions.
 */
template <int N>
auto bbox(const Path<N>& in)
{
  auto first = Position<N>(in.rank()); // FIXME first = in.front();
  for (int i = 0; i < in.rank(); ++i) {
    first[i] = in(0, i);
  }
  auto out = Box(first, first);

  for (int i = 0; i < in.size(); ++i) {
    for (int j = 0; j < in.rank(); ++j) {
      auto& start_j = out.start(j);
      auto& stop_j = out.stop(j);
      auto p_j = in(i, j);
      start_j = std::min(start_j, p_j);
      stop_j = std::max(stop_j, p_j + 1);
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
    Kokkos::deep_copy(m_view.base(), h.base()); // FIXME as method? in copy_from/to()?
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

  Image<int, 2, ImageView<int, 2, TSpace>> m_view;
  TFunc m_func;
};

/**
 * @ingroup regions
 * @relatesalso Path
 * @brief Apply a function to each element of the domain.
 * @tparam TSpace The execution space
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, int N, typename TFunc>
void for_each(const std::string& label, Path<N> region, TFunc func)
{
  for_each<TSpace>(label, Slice(0, region.ssize()), ExpandPath<TSpace, N, TFunc>(region, func));
}

} // namespace Linx

#endif
