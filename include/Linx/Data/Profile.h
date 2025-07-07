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
 * @brief Resizable sequence of elements along a trajectory.
 * 
 * Elements are accessed with a 1D index:
 * the i-th element of the profile is the element of the parent mapping at the i-th position of the trajectory.
 * For performance, the trajectory is not stored, as opposed to a trajectory-based patch.
 * The domain of the profile is a 1D slice.
 * 
 * Elements can be added up to a given maximum, fixed capacity.
 */
template <Strided TParent> // FIXME must work with all mappings
class Profile : public DataMixin<typename TParent::value_type, typename TParent::Arithmetic, Profile<TParent>> {
public:

  using Parent = TParent; ///< The parent mapping
  static constexpr auto n = 1; ///< The rank
  using value_type = Parent::value_type; ///< The value type
  using element_type = std::remove_cvref_t<value_type>; ///< The element type
  using reference = value_type&; ///< The reference type
  using memory_space = typename TParent::memory_space; ///< The parent memory space
  using execution_space = typename TParent::execution_space; ///< The parent execution space

  /**
   * @brief Constructor.
   * @param parent The parent mapping
   * @param capacity The maximum size of the sequence
   */
  Profile(const Parent& parent, std::integral auto capacity) :
      m_parent(parent),
      m_offsets("Profile offsets", capacity),
      m_size("Profile size")
  {}

  /**
   * @brief Region-based constructor.
   */
  Profile(const Parent& parent, const NotConvertibleTo<std::size_t> auto& region) : Profile(parent, region.size())
  {
    assign(region);
  }

  /**
   * @brief Assign the positions of a region.
   */
  void assign(const auto& region) const
  {
    for_each<execution_space>( // FIXME keep order?
        "assign",
        region,
        KOKKOS_CLASS_LAMBDA(std::integral auto... position) { emplace_back(position...); });
  }

  /**
   * @brief Rank: 1.
   */
  static constexpr auto rank()
  {
    return 1;
  }

  /**
   * @brief Number of elements.
   */
  auto size() const
  {
    return Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), m_size)();
  }

  /**
   * @brief Index range.
   */
  auto domain() const
  {
    return Slice(0, size());
  }

  /**
   * @brief Reference to the i-th element.
   */
  KOKKOS_INLINE_FUNCTION reference operator()(std::integral auto i) const
  {
    return m_parent.data()[m_offsets(i)]; // FIXME .origin()?
  }

  /**
   * @brief Reference to the i-th element.
   */
  KOKKOS_INLINE_FUNCTION reference operator[](std::integral auto i) const
  {
    return m_parent.data()[m_offsets(i)]; // FIXME .origin()?
  }

  /**
   * @brief Append an offset.
   */
  KOKKOS_INLINE_FUNCTION void push_back(std::ptrdiff_t offset) const
  {
    auto index = Kokkos::atomic_fetch_add(&m_size(), 1);
    m_offsets(index) = offset;
  }

  /**
   * @brief Append and get the offset of a position.
   */
  KOKKOS_INLINE_FUNCTION std::ptrdiff_t emplace_back(std::integral auto... position) const
  {
    auto out = offset_from_origin(m_parent, position...); // FIXME no need to require Strided
    push_back(out);
    return out;
  }

private:

  Parent m_parent; ///< The parent data container
  Sequence<std::ptrdiff_t, -1, SequenceContainer<std::ptrdiff_t, -1, memory_space>>
      m_offsets; ///< The offsets in the parent
  Kokkos::View<std::size_t, memory_space> m_size; ///< The profile size
};

namespace Impl {

template <typename TIn, typename TPred>
struct FilterOffsets {
  TIn m_in;
  TPred m_pred;
  Profile<TIn> m_out;
  KOKKOS_INLINE_FUNCTION void operator()(auto... position) const
  {
    const auto* ptr = &m_in(position...);
    if (m_pred(*ptr)) {
      m_out.push_back(ptr - m_in.data()); // FIXME .origin()
    }
  }
};

} // namespace Impl

/**
 * @relatesalso Profile
 * @brief Select the positions where some predicate over an input container's elements holds.
 */
template <Strided TIn, typename TPred>
Profile<TIn> filter(const TIn& in, TPred pred)
{
  auto size = transform_reduce(
      "filter size",
      KOKKOS_LAMBDA(const typename TIn::value_type& e) { return pred(e) ? 1 : 0; },
      Add(),
      in); // FIXME in.count_if(pred)
  auto out = Profile<TIn>(in, size);
  for_each<typename TIn::execution_space>("filter", in.domain(), Impl::FilterOffsets<TIn, TPred> {in, pred, out});
  return out;
}

} // namespace Linx

#endif
