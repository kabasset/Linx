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

  Profile(const Parent& parent, std::integral auto capacity) :
      m_parent(parent),
      m_offsets("Profile offsets", capacity),
      m_size("Profile size")
  {}

  Profile(const Parent& parent, const NotConvertibleTo<std::size_t> auto& region) : Profile(parent, region.size())
  {
    const auto& offsets_on_host = on_host(m_offsets);
    auto it = offsets_on_host.begin();
    for_each<Kokkos::Serial>("Profile", region, [&](std::integral auto... position) {
      *it = offset_from_origin(m_parent, position...);
      ++it;
    });
    Kokkos::deep_copy(m_offsets.container(), offsets_on_host.container());
    m_size() = m_offsets.size();
  }

  KOKKOS_INLINE_FUNCTION auto rank() const
  {
    return m_parent.rank();
  }

  KOKKOS_INLINE_FUNCTION auto size() const
  {
    return m_size();
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
  /**
   * @brief Append a position.
   */
  KOKKOS_INLINE_FUNCTION void push_back(std::ptrdiff_t offset) const
  {
    auto index = Kokkos::atomic_fetch_add(&m_size(), 1);
    m_offsets(index) = offset;
  }

private:

  // FIXME use m_parent memory_space
  Parent m_parent; ///< The parent data container
  Sequence<std::ptrdiff_t, -1> m_offsets; ///< The offsets in the parent
  Kokkos::View<std::size_t> m_size; ///< The profile size
};

/**
 * @relatesalso Profile
 * @brief Select the positions where some predicate over an input container's elements holds.
 */
template <Strided TIn, typename TPred>
auto filter(const TIn& in, TPred pred)
{
  auto size = transform_reduce(
      "filter size",
      KOKKOS_LAMBDA(const typename TIn::value_type& e) { return pred(e) ? 1 : 0; },
      Add(),
      in); // FIXME in.count_if(pred)
  auto out = Profile<TIn>(in, size);
  for_each<typename TIn::execution_space>(
      "filter",
      in.domain(),
      KOKKOS_LAMBDA(auto... position) {
        const auto* ptr = &in(position...);
        if (pred(*ptr)) {
          out.push_back(ptr - in.data());
        }
      });
  return out;
}

} // namespace Linx

#endif
