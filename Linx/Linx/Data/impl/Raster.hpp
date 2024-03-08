// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXDATA_IMPL_RASTER_HPP
#define _LINXDATA_IMPL_RASTER_HPP

#include "Linx/Data/Line.h"
#include "Linx/Data/Raster.h"

#include <functional> // multiplies
#include <numeric> // accumulate

namespace Linx {

/// @cond INTERNAL
namespace Internal {

/**
 * @brief nD-index recursive implementation.
 * @tparam N The raster dimension.
 * @tparam i The dimension of the current recursion step, should be initialized with `N - 1`.
 */
template <Index N, Index I = N - 1>
struct IndexRecursionImpl {
  /**
   * @brief Index of given position in given shape for Raster::index.
   */
  static Index index(const Position<N>& shape, const Position<N>& pos)
  {
    return std::get<N - 1 - I>(pos.container()) +
        std::get<N - 1 - I>(shape.container()) * IndexRecursionImpl<N, I - 1>::index(shape, pos);
  }
};

/**
 * @brief Terminal case: dimension 0.
 */
template <Index N>
struct IndexRecursionImpl<N, 0> {
  /**
   * @brief pos[N - 1]
   */
  static Index index(const Position<N>& shape, const Position<N>& pos)
  {
    (void)(shape);
    return std::get<N - 1>(pos.container());
  }
};

/**
 * @brief Variable dimension case.
 */
template <Index I>
struct IndexRecursionImpl<-1, I> {
  /**
   * @brief pos[0] + shape[1] * (pos[1] + shape[2] * (pos[2] + shape[3] * (...)))
   */
  static Index index(const Position<-1>& shape, const Position<-1>& pos)
  {
    const auto n = shape.size();
    SizeError::may_throw(pos.size(), n);
    Index res = 0;
    for (std::size_t j = 0; j < shape.size(); ++j) {
      res = pos[n - 1 - j] + shape[n - 1 - j] * res;
    }
    return res;
  }
};

} // namespace Internal
/// @endcond

// Raster

template <typename T, Index N, typename THolder>
const Position<N>& Raster<T, N, THolder>::shape() const
{
  return m_shape;
}

template <typename T, Index N, typename THolder>
Box<N> Raster<T, N, THolder>::domain() const
{
  return Box<N>::from_shape(m_shape);
}

template <typename T, Index N, typename THolder>
inline Index Raster<T, N, THolder>::dimension() const
{
  return m_shape.size();
}

template <typename T, Index N, typename THolder>
template <Index I>
inline Index Raster<T, N, THolder>::length() const
{
  return std::get<I>(m_shape.container());
}

template <typename T, Index N, typename THolder>
inline Index Raster<T, N, THolder>::index(const Position<N>& pos) const
{
  return Internal::IndexRecursionImpl<N>::index(m_shape, pos);
}

template <typename T, Index N, typename THolder>
inline const T& Raster<T, N, THolder>::operator[](const Position<N>& pos) const
{
  return (*this)[index(pos)];
}

template <typename T, Index N, typename THolder>
inline T& Raster<T, N, THolder>::operator[](const Position<N>& pos)
{
  return const_cast<T&>(const_cast<const Raster&>(*this)[pos]);
}

template <typename T, Index N, typename THolder>
inline const T& Raster<T, N, THolder>::at(const Position<N>& pos) const
{
  auto bounded_pos = pos;
  for (Index i = 0; i < dimension(); ++i) {
    auto& b = bounded_pos[i];
    const auto& s = m_shape[i];
    OutOfBoundsError::may_throw("pos[" + std::to_string(i) + "]", b, {-s, s - 1});
    if (b < 0) {
      b += s;
    }
  }
  return operator[](bounded_pos);
}

template <typename T, Index N, typename THolder>
inline T& Raster<T, N, THolder>::at(const Position<N>& pos)
{
  return const_cast<T&>(const_cast<const Raster&>(*this).at(pos));
}

template <typename T, Index N, typename THolder>
template <Index M>
PtrRaster<const T, M> Raster<T, N, THolder>::slice(const Box<N>& region) const
{
  // FIXME resolve
  if (not is_contiguous<M>(region)) {
    throw Exception("Cannot slice: Box is not contiguous."); // FIXME clarify
  }
  const auto& f = region.front();
  const auto& b = region.back();
  Position<M> reduced(M);
  for (Index i = 0; i < M; ++i) {
    reduced[i] = b[i] - f[i] + 1;
  }
  return PtrRaster<const T, M> {reduced, &operator[](region.front())};
}

template <typename T, Index N, typename THolder>
template <Index M>
PtrRaster<T, M> Raster<T, N, THolder>::slice(const Box<N>& region)
{
  if (not is_contiguous<M>(region)) {
    throw Exception("Cannot slice: Box is not contiguous."); // FIXME clarify
  }
  const auto& f = region.front();
  const auto& b = region.back();
  Position<M> reduced(M);
  for (Index i = 0; i < M; ++i) {
    reduced[i] = b[i] - f[i] + 1;
  }
  // FIXME duplication
  return PtrRaster<T, M> {reduced, &operator[](region.front())};
}

template <typename T, Index N, typename THolder>
typename Raster<T, N, THolder>::ConstChunk Raster<T, N, THolder>::chunk(Index front, Index back) const
{
  const auto last = dimension() - 1;
  auto f = Position<N>::zero(); // FIXME support N = -1
  auto b = shape() - 1;
  f[last] = front;
  b[last] = back;
  return {*this, Box<N>(f, b)};
}

template <typename T, Index N, typename THolder>
typename Raster<T, N, THolder>::Chunk Raster<T, N, THolder>::chunk(Index front, Index back)
{
  const auto last = dimension() - 1;
  auto f = Position<N>::zero(); // FIXME support N = -1
  auto b = shape() - 1;
  f[last] = front;
  b[last] = back;
  return {*this, Box<N>(f, b)};
}

template <typename T, Index N, typename THolder>
typename Raster<T, N, THolder>::ConstSection Raster<T, N, THolder>::section(Index index) const
{
  const auto last = dimension() - 1;
  auto f = Position<N>::zero(); // FIXME support -1
  auto b = shape() - 1;
  f[last] = index;
  b[last] = index;
  return slice<ConstSection::Dimension>(Box<N>(f, b));
}

template <typename T, Index N, typename THolder>
typename Raster<T, N, THolder>::Section Raster<T, N, THolder>::section(Index index)
{
  const auto last = dimension() - 1;
  auto f = Position<N>::zero(); // FIXME support -1
  auto b = shape() - 1;
  f[last] = index;
  b[last] = index;
  return slice<Section::Dimension>(Box<N>(f, b));
}

template <typename T, Index N, typename THolder>
typename Raster<T, N, THolder>::ConstRow
Raster<T, N, THolder>::row(const Position<Raster<T, N, THolder>::OneLessDimension>& position) const
{
  Position<N> f(dimension());
  for (Index i = 1; i < dimension(); ++i) {
    const auto p = position[i - 1];
    f[i] = p >= 0 ? p : length(i) + p;
  }
  return {*this, Line<0, N>(f, length(0) - 1)};
}

template <typename T, Index N, typename THolder>
typename Raster<T, N, THolder>::Row
Raster<T, N, THolder>::row(const Position<Raster<T, N, THolder>::OneLessDimension>& position)
{
  Position<N> f(dimension());
  for (Index i = 1; i < dimension(); ++i) {
    const auto p = position[i - 1];
    f[i] = p >= 0 ? p : length(i) + p;
  }
  return {*this, Line<0, N>(f, length(0) - 1)};
}

template <typename T, Index N, typename THolder>
template <Index I>
Raster<T, N, THolder>::ConstProfile<I>
Raster<T, N, THolder>::profile(const Position<Raster<T, N, THolder>::OneLessDimension>& position) const
{
  Position<N> f(dimension());
  for (Index i = 0; i < I; ++i) {
    const auto p = position[i];
    f[i] = p >= 0 ? p : length(i) + p;
  }
  f[I] = 0;
  for (Index i = I + 1; i < dimension(); ++i) {
    const auto p = position[i - 1];
    f[i] = p >= 0 ? p : length(i) + p;
  }
  return {*this, Line<I, N>(f, length<I>() - 1)};
}

template <typename T, Index N, typename THolder>
template <Index I>
Raster<T, N, THolder>::Profile<I>
Raster<T, N, THolder>::profile(const Position<Raster<T, N, THolder>::OneLessDimension>& position)
{
  Position<N> f(dimension());
  for (Index i = 0; i < I; ++i) {
    f[i] = position[i];
  }
  f[I] = 0;
  for (Index i = I + 1; i < dimension(); ++i) {
    f[i] = position[i - 1];
  }
  return {*this, Line<I, N>(f, length<I>() - 1)};
}

template <typename T, Index N, typename THolder>
template <Index M>
bool Raster<T, N, THolder>::is_contiguous(const Box<N>& region) const
{
  const auto& f = region.front();
  const auto& b = region.back();
  for (Index i = 0; i < M - 1; ++i) {
    if (f[i] != 0 || b[i] != m_shape[i] - 1) { // Doesn't span across the full axis => index jump
      return false;
    }
  }
  for (Index i = M; i < dimension(); ++i) {
    if (b[i] != f[i]) { // Not flat along axis i >= M => dimension >= M
      return false;
    }
  }
  return true;
}

} // namespace Linx

#endif
