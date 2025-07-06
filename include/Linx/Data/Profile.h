// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_PROFILE_H
#define LINX_DATA_PROFILE_H

#include "Linx/Base/mixins/Data.h"
#include "Linx/Base/mixins/Strided.h"

#include <concepts>
#include <string>

namespace Linx {

/**
 * @brief A mapping between a sequence of 1D indices and a sequence of array elements along a path.
 */
template <Strided TParent>
class Profile : public DataMixin<typename TParent::value_type, typename TParent::Arithmetic, Profile<TParent>> {
public:

  using Parent = TParent;
  static constexpr auto n = TParent::n;
  using value_type = Parent::value_type;
  using element_type = std::remove_cvref_t<value_type>;
  using reference = value_type&;
  using execution_space = typename TParent::execution_space;

  Profile(const Parent& parent, const auto& region) : m_parent(parent), m_offsets("Profile offsets", region.size())
  {
    const auto& offsets_on_host = on_host(m_offsets);
    auto it = offsets_on_host.begin();
    for_each<Kokkos::Serial>("Profile", region, [&](std::integral auto... position) {
      *it = offset_from_origin(m_parent, position...);
      ++it;
    });
    Kokkos::deep_copy(m_offsets.container(), offsets_on_host.container());
  }

  KOKKOS_INLINE_FUNCTION auto rank() const
  {
    return m_parent.rank();
  }

  KOKKOS_INLINE_FUNCTION auto size() const
  {
    return m_offsets.size();
  }

  KOKKOS_INLINE_FUNCTION auto domain() const
  {
    return Slice(0, size());
  }

  KOKKOS_INLINE_FUNCTION reference operator()(std::integral auto i) const
  {
    return m_parent.data()[m_offsets(i)]; // FIXME .origin()?
  }

  KOKKOS_INLINE_FUNCTION reference operator[](std::integral auto i) const
  {
    return m_parent.data()[m_offsets(i)]; // FIXME .origin()?
  }

private:

  Parent m_parent; ///< The parent data container
  Sequence<std::ptrdiff_t, -1> m_offsets; ///< The offsets in the parent
};

/**
 * @relatesalso Profile
 * @brief Select the positions where some predicate over an input container's elements holds.
 */
template <Strided TIn, typename TPred>
auto filter(const TIn& in, TPred pred)
{
  const auto& in_on_host = on_host(in);
  auto raster = Raster<bool, TIn::n>("raster", in.shape()).generate("pred", pred, in_on_host);
  auto path = Path<TIn::n>("Path", sum(raster));
  for_each<Kokkos::Serial>("filter", raster.domain(), [&](auto... position) {
    if (raster(position...)) {
      path.push_back(position...);
    }
  });
  return Profile(in, path);
}

} // namespace Linx

#endif
