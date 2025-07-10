// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_PROFILE_H
#define LINX_DATA_PROFILE_H

#include "Linx/Base/mixins/Data.h"
#include "Linx/Base/mixins/Strided.h"
#include "Linx/Data/Sequence.h"

#include <concepts>
#include <string>

namespace Linx {

template <typename T>
class ProfileIterator {
public:

  using iterator_category = std::random_access_iterator_tag;
  using difference_type = int;
  using value_type = T;
  using pointer = T*;
  using reference = T&;

  /**
   * @brief Constructor.
   * 
   * @param data The reference data pointer
   * @param offsets The sequence of address offsets
   */
  KOKKOS_INLINE_FUNCTION explicit ProfileIterator(T* data, const std::ptrdiff_t* begin) :
      m_data(data),
      m_begin(begin),
      m_it(begin)
  {}

  KOKKOS_INLINE_FUNCTION ProfileIterator(const ProfileIterator& rhs) :
      m_data(rhs.m_data),
      m_begin(rhs.m_begin),
      m_it(rhs.m_it)
  {}

  KOKKOS_INLINE_FUNCTION ProfileIterator& operator=(const ProfileIterator& rhs)
  {
    m_data = rhs.m_data;
    m_begin = rhs.m_begin;
    m_it = rhs.m_it;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION void reset(T* data)
  {
    m_data = data;
    m_it = m_begin;
  }

  KOKKOS_INLINE_FUNCTION reference operator[](int i) const
  {
    return m_data[m_begin[i]];
  }

  KOKKOS_INLINE_FUNCTION reference operator*() const
  {
    return m_data[*m_it];
  }

  KOKKOS_INLINE_FUNCTION pointer operator->() const
  {
    return m_data + *m_it;
  }

  KOKKOS_INLINE_FUNCTION ProfileIterator& operator++()
  {
    ++m_it;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION ProfileIterator operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  KOKKOS_INLINE_FUNCTION ProfileIterator& operator+=(int i)
  {
    m_it += i;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION ProfileIterator operator+(int i) const
  {
    auto out = *this;
    out.m_it += i;
    return out;
  }

  KOKKOS_INLINE_FUNCTION ProfileIterator& operator--()
  {
    --m_it;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION ProfileIterator operator--(int)
  {
    auto out = *this;
    --(*this);
    return out;
  }

  KOKKOS_INLINE_FUNCTION ProfileIterator& operator-=(int i)
  {
    m_it -= i;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION ProfileIterator operator-(int i) const
  {
    auto out = *this;
    out.m_it -= i;
    return out;
  }

  KOKKOS_INLINE_FUNCTION difference_type operator-(const ProfileIterator& rhs) const
  {
    return m_it - rhs.m_it;
  }

  KOKKOS_INLINE_FUNCTION bool operator==(const ProfileIterator& rhs) const
  {
    return m_it == rhs.m_it;
  }

  KOKKOS_INLINE_FUNCTION bool operator!=(const ProfileIterator& rhs) const
  {
    return m_it != rhs.m_it;
  }

  KOKKOS_INLINE_FUNCTION bool operator==(const std::ptrdiff_t* it) const
  {
    return m_it == it;
  }

  KOKKOS_INLINE_FUNCTION bool operator!=(const std::ptrdiff_t* it) const
  {
    return m_it != it;
  }

private:

  T* m_data; ///< The reference data
  const std::ptrdiff_t* m_begin; ///< The beginning offset iterator
  const std::ptrdiff_t* m_it; ///< The current offset iterator
};

template <typename T>
class ProfileSpan {
public:

  KOKKOS_INLINE_FUNCTION ProfileSpan(T* data, const std::ptrdiff_t* begin, const std::ptrdiff_t* end) :
      m_data(data),
      m_begin(begin),
      m_end(end)
  {}

  KOKKOS_INLINE_FUNCTION auto begin() const
  {
    return ProfileIterator<T>(m_data, m_begin);
  }

  KOKKOS_INLINE_FUNCTION auto end() const
  {
    return m_end;
  }

  KOKKOS_INLINE_FUNCTION auto ssize() const
  {
    return m_end - m_begin;
  }

  KOKKOS_INLINE_FUNCTION T& operator[](int i) const
  {
    return m_data[m_begin[i]];
  }

private:

  T* m_data;
  const std::ptrdiff_t* m_begin;
  const std::ptrdiff_t* m_end;
};

namespace Impl {

template <typename TProfile>
struct EmplaceProfile {
  KOKKOS_INLINE_FUNCTION void operator()(std::integral auto... position) const
  {
    m_profile.emplace_back(position...);
  }
  TProfile m_profile;
};

} // namespace Impl

/**
 * @brief Resizable sequence of elements specified as offset from a reference memory address.
 * 
 * Elements are accessed with a 1D index:
 * the i-th element of the profile is the i-th inserted element of the parent mapping.
 * For performance, as opposed to a trajectory-based patch, the positions are not stored.
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
  using pointer = value_type&; ///< The pointer type
  using iterator = ProfileIterator<value_type>; ///< The iterator type
  using const_iterator = ProfileIterator<const value_type>; ///< The read-only iterator type
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
    // FIXME keep order?
    for_each<execution_space>("assign", region, Impl::EmplaceProfile {*this});
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
   * @brief Maximum number of elements.
   */
  auto capacity() const
  {
    return m_offsets.size();
  }

  /**
   * @brief Parent data container.
   */
  const Parent& parent() const
  {
    return m_parent;
  }

  /**
   * @brief Index range.
   */
  auto domain() const
  {
    return Slice(0, size());
  }

  /**
   * @brief Iterator to the beginning.
   */
  KOKKOS_INLINE_FUNCTION iterator begin() const
  {
    return iterator(m_parent.data(), m_offsets.data());
  }

  /**
   * @brief Iterator to the end.
   */
  KOKKOS_INLINE_FUNCTION iterator end() const
  {
    return iterator(m_parent.data(), m_offsets.data() + m_size());
  }

  /**
   * @brief Read-only iterator to the beginning.
   */
  KOKKOS_INLINE_FUNCTION const_iterator cbegin() const
  {
    return const_iterator(m_parent.data(), m_offsets.data());
  }

  /**
   * @brief Read-only iterator to the end.
   */
  KOKKOS_INLINE_FUNCTION const_iterator cend() const
  {
    return const_iterator(m_parent.data(), m_offsets.data() + m_size());
  }

  /**
   * @brief Span relative to a given position.
   */
  KOKKOS_INLINE_FUNCTION ProfileSpan<value_type> shifted_span(std::integral auto... position) const
  {
    return ProfileSpan<value_type>(&m_parent(position...), m_offsets.data(), m_offsets.data() + m_size());
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
   * @return The index of the appended element.
   */
  KOKKOS_INLINE_FUNCTION std::size_t push_back(std::ptrdiff_t offset) const
  {
    std::size_t index = Kokkos::atomic_fetch_add(&m_size(), 1);
    m_offsets(index) = offset;
    return index;
  }

  /**
   * @brief Append and get the offset of a position.
   * @return The index of the appended element.
   */
  KOKKOS_INLINE_FUNCTION std::size_t emplace_back(std::integral auto... position) const
  {
    auto out = offset_from_origin(m_parent, position...); // FIXME no need to require Strided
    return push_back(out);
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
