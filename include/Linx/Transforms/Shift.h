// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_SHIFT_H
#define LINX_TRANSFORMS_SHIFT_H

#include "Linx/Base/Functional.h"
#include "Linx/Base/Types.h"
#include "Linx/Data/Image.h"
#include "Linx/Data/Sequence.h"

#include <Kokkos_Core.hpp>

namespace Linx {

/**
 * @brief A shifted view of an image.
 */
template <typename TParent>
class Shift : public DataMixin<typename TParent::value_type, typename TParent::Arithmetic, Shift<TParent>> {
public:

  static constexpr int n = TParent::Domain::n; ///< The dimension parameter
  using Parent = TParent; ///< The parent
  using Domain = Parent::Domain; ///< The domain

  using memory_space = typename Parent::memory_space;
  using execution_space = typename Parent::execution_space;

  using value_type = typename Parent::value_type; ///< The value type
  using element_type = std::remove_cv_t<value_type>; ///< The element typ
  using reference = typename Parent::reference; ///< The reference type
  using difference_type = std::ptrdiff_t; ///< The index difference type

  /**
   * @brief Constructor.
   */
  Shift() : m_parent(nullptr), m_offset {} {}

  /**
   * @copydoc Shift()
   */
  Shift(const Parent& parent, std::integral auto... offset) : m_parent(parent), m_offset {offset...} {}

  /**
   * @copydoc Shift()
   */
  Shift(const Parent& parent, Position<n> offset) : m_parent(parent), m_offset {}
  {
    std::ranges::copy(offset, m_offset.data());
  }

  /**
   * @brief The parent.
   */
  KOKKOS_INLINE_FUNCTION const Parent& parent() const
  {
    return m_parent;
  }

  /**
   * @brief The offset.
   */
  Position<n> offset() const
  {
    return Position<n>("offset", m_offset.data(), m_offset.data() + m_offset.size());
    // FIXME implement Sequence::rank() -> 1 and replace m_offset.size() with m_parent.rank()
  }

  /**
   * @brief The domain.
   */
  auto domain() const
  {
    return box(m_parent.domain()) + offset(); // FIXME rm box() by implementing Slice::operator+
  }

  /**
   * @brief The underlying container.
   */
  KOKKOS_INLINE_FUNCTION const auto& container() const
  {
    return root(*this).container();
  }

  /**
   * @brief The domain size.
   */
  KOKKOS_INLINE_FUNCTION auto size() const
  {
    return m_parent.size();
  }

  /**
   * @brief Address offset between the origin element and the element at given indices.
   * 
   * By definition, `shift.distance_from_origin(0, 0, ...)` is 0,
   * while `shift.parent().distance_from_origin(0, 0, ...)` is generally not 0,
   * since the origin is shifted.
   */
  KOKKOS_INLINE_FUNCTION difference_type distance_from_origin(std::integral auto... indices) const
  {
    return distance_impl(forward_as_tuple(indices...), std::make_index_sequence<sizeof...(indices)>());
  }

  /**
   * @brief Forward to parent's `operator()` after shifting.
   */
  KOKKOS_INLINE_FUNCTION reference operator()(std::integral auto... is) const
  {
    return at_impl(forward_as_tuple(is...), std::make_index_sequence<sizeof...(is)>());
  }

private:

  /**
   * @brief Helper method to unroll indices.
   */
  template <std::size_t... Is>
  KOKKOS_INLINE_FUNCTION difference_type distance_impl(const auto& indices, std::index_sequence<Is...>) const
  {
    return ((get<Is>(indices) * m_parent.stride(Is)) + ...);
  }

  /**
   * @brief Helper method to unroll indices.
   */
  template <std::size_t... Is>
  KOKKOS_INLINE_FUNCTION reference at_impl(const auto& indices, std::index_sequence<Is...>) const
  {
    return m_parent((get<Is>(indices) - m_offset[Is])...);
  }

private:

  static constexpr int kokkos_max_dyn_rank = 7;
  static constexpr int max_rank = (n == -1 ? kokkos_max_dyn_rank : n);

  Parent m_parent; ///< The parent
  Kokkos::Array<Index, max_rank> m_offset; ///< The offset
};

template <typename T>
struct IsOffset : std::false_type {};
template <typename T>
struct IsOffset<Shift<T>> : std::true_type {}; // FIXME rename as Offset?
// FIXME IsOffset<Patch<T, Box/Slice>> : std::true_type {};

template <typename T>
constexpr bool is_offset()
{
  return IsOffset<T>::value;
}

template <typename T>
concept AnyShift = is_specialization<Shift, T>;

/**
 * @relatesalso Shift
 * @brief Get the root data container.
 */
KOKKOS_INLINE_FUNCTION const auto& root(const AnyShift auto& shift)
{
  return root(shift.parent());
}

template <typename TParent>
auto as_readonly(const Shift<TParent>& in)
{
  return Shift(as_readonly(in.parent()), in.offset());
}

template <typename TSpace, typename TParent>
decltype(auto) on_device(const Shift<TParent>& in)
{
  return Shift(on_device<TSpace>(in.parent()), in.offset());
}

template <typename TParent>
decltype(auto) on_host(const Shift<TParent>& in)
{
  return on_device<Kokkos::HostSpace>(in);
}

/**
 * @brief Align a contiguous-domain 1D data container along an axis, reshaping it into an ND image.
 * @tparam I The axis to align the array along
 * @tparam N The rank of the output image (-1 is not supported)
 * 
 * The input can be a sequence or a 1D image, possibly offset.
 * The output is an image or an offset image.
 */
template <Index I, Index N = I + 1, typename TIn>
auto along(const TIn& in)
{
  const auto& r = root(in);
  auto shape = Position<N>("shape").fill(1);
  shape[I] = r.size();
  Image<typename TIn::element_type, N> out(r.label(), shape); // FIXME on_device<TIn::execution_space>
  const auto& out_on_host = on_host(out);
  for (Index i = 0; i < in.ssize(); ++i) {
    auto p = Position<N>("p");
    p[I] = i;
    out_on_host[p] = r[i];
  }
  Kokkos::deep_copy(out.container(), out_on_host.container());
  if constexpr (not is_offset<TIn>()) {
    return out;
  } else {
    auto offset = Position<N>("offset");
    offset[I] = in.domain().start(0);
    return Shift(out, offset);
  }
}

} // namespace Linx

#endif
