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
class Shift : public DataMixin<typename TParent::value_type, EuclidArithmetic, Shift<TParent>> {
public:

  static constexpr int n = TParent::Domain::n; ///< The dimension parameter
  using Parent = TParent; ///< The parent
  using Domain = Parent::Domain; ///< The domain

  using memory_space = typename Parent::memory_space;
  using execution_space = typename Parent::execution_space;

  using value_type = typename Parent::value_type; ///< The value type
  using reference = typename Parent::reference; ///< The reference type

  /**
   * @brief Constructor.
   */
  Shift() : m_parent(nullptr), m_offset("offset", {}) {}

  /**
   * @copydoc Shift()
   */
  Shift(const Parent& parent, std::integral auto... offset) : m_parent(parent), m_offset("offset", {offset...}) {}

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
    Position<n> out("offset", m_offset.size());
    Kokkos::deep_copy(out.container(), m_offset.container());
    return out;
  }

  /**
   * @brief The domain.
   */
  auto domain() const
  {
    return m_parent.domain() + offset();
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
  KOKKOS_INLINE_FUNCTION reference at_impl(const auto& indices, std::index_sequence<Is...>) const
  {
    return m_parent((get<Is>(indices) + m_offset[Is])...);
  }

private:

  Parent m_parent; ///< The parent
  Sequence<Index, n> m_offset; ///< The offset
};

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

} // namespace Linx

#endif
