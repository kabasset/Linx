// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_PROFILE_H
#define LINX_DATA_PROFILE_H

#include "Linx/Base/mixins/Data.h"

#include <concepts>
#include <string>

namespace Linx {

/**
 * @brief A mapping between a sequence of 1D indices and a sequence of array elements along a path.
 */
template <typename TParent>
class Profile : public DataMixin<typename TParent::value_type, typename TParent::Arithmetic, Profile<TParent>> {
public:

  using Parent = TParent;
  static constexpr auto n = TParent::n;
  using value_type = Parent::value_type;
  using element_type = std::remove_cvref_t<value_type>;
  using reference = value_type&;
  using execution_space = typename TParent::execution_space;

  Profile(TParent parent, std::size_t capacity) : m_parent(parent), m_path("Profile path", capacity) {}

  KOKKOS_INLINE_FUNCTION auto rank() const
  {
    return m_path.rank();
  }

  KOKKOS_INLINE_FUNCTION auto size() const
  {
    return m_path.size();
  }

  KOKKOS_INLINE_FUNCTION auto domain() const
  {
    return Slice(0, size());
  }

  KOKKOS_INLINE_FUNCTION const auto& path() const
  {
    return m_path;
  }

  KOKKOS_INLINE_FUNCTION reference operator()(std::integral auto i) const
  {
    return at_impl(i, std::make_index_sequence<n>());
  }

  KOKKOS_INLINE_FUNCTION reference operator[](std::integral auto i) const
  {
    return at_impl(i, std::make_index_sequence<n>());
  }

  KOKKOS_INLINE_FUNCTION void push_back(std::integral auto... position) const
  {
    m_path.push_back(position...); // FIXME push_back?
  }

private:

  template <std::size_t... Is>
  KOKKOS_INLINE_FUNCTION decltype(auto) at_impl(auto i, std::index_sequence<Is...>) const
  {
    return m_parent(m_path(i, Is)...);
  }

private:

  Parent m_parent;
  Path<n> m_path;
};

/**
 * @relatesalso Profile
 * @brief Select the positions where some predicate over an input container's elements holds.
 */
template <typename TIn, typename TPred>
auto filter(const TIn& in, TPred pred)
{
  const auto& in_on_host = on_host(in);
  auto raster = Raster<bool, TIn::n>("raster", in.shape()).generate("pred", pred, in_on_host);
  auto out = Profile(in, sum(raster));
  for_each<Kokkos::Serial>("filter", in.domain(), [&](auto... position) {
    if (pred(in(position...))) {
      out.push_back(position...);
    }
  });
  return out;
}

} // namespace Linx

#endif
