// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_PATCH_H
#define LINX_DATA_PATCH_H

#include "Linx/Base/Functional.h"
#include "Linx/Base/Types.h"
#include "Linx/Data/Image.h"
#include "Linx/Data/Sequence.h"

#include <Kokkos_Core.hpp>

namespace Linx {

/**
 * @brief An image patch.
 * 
 * A patch is a restriction of an image to some domain.
 * As opposed to an image slice, an image patch always has the same rank as the image, and its domain is the input domain.
 */
template <typename TParent, typename TDomain>
class Patch : public DataMixin<typename TParent::value_type, typename TParent::Arithmetic, Patch<TParent, TDomain>> {
public:

  static constexpr int n = TDomain::n; ///< The dimension parameter
  using Parent = TParent; ///< The parent, which may be a patch
  using Domain = TDomain; ///< The domain

  using memory_space = typename Parent::memory_space;
  using execution_space = typename Parent::execution_space;

  using value_type = typename Parent::value_type; ///< The value type
  using reference = typename Parent::reference; ///< The reference type

  /**
   * @brief Default constructor.
   */
  Patch() : m_parent(nullptr), m_domain() {}

  /**
   * @brief Constructor.
   */
  Patch(const Parent& parent, Domain region) : m_parent(parent), m_domain(LINX_MOVE(region)) {}

  /**
   * @brief The parent.
   */
  KOKKOS_INLINE_FUNCTION const Parent& parent() const
  {
    return m_parent;
  }

  /**
   * @brief The domain.
   */
  KOKKOS_INLINE_FUNCTION const Domain& domain() const
  {
    return m_domain;
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
    return domain().size();
  }

  /**
   * @brief Forward to parent's `operator[]`.
   */
  KOKKOS_INLINE_FUNCTION reference operator[](auto&& arg) const
  {
    return m_parent[LINX_FORWARD(arg)];
  }

  /**
   * @brief Forward to parent's `operator()`.
   */
  KOKKOS_INLINE_FUNCTION reference operator()(auto&&... args) const
  {
    return m_parent(LINX_FORWARD(args)...);
  }

  /**
   * @brief Get the element at a given domain-local position.
   * 
   * The arguments are forwarded to the domain,
   * such that the method returns `parent[domain(args...)]`.
   */
  reference local(auto&&... args) const // FIXME as KOKKOS_INLINE_FUNCTION
  {
    return m_parent[m_domain(LINX_FORWARD(args)...)];
  }

  /**
   * @brief Shift the patch by a given vector.
   */
  KOKKOS_INLINE_FUNCTION Patch& operator>>=(const auto& vector)
  {
    m_domain += vector;
    return *this;
  }

  /**
   * @brief Shift the patch by the opposite of a given vector.
   */
  KOKKOS_INLINE_FUNCTION Patch& operator<<=(const auto& vector)
  {
    m_domain -= vector;
    return *this;
  }

  /**
   * @brief Shift the patch by given indices.
   */
  KOKKOS_INLINE_FUNCTION Patch& shift(auto... is)
  {
    m_domain.add(is...);
    return *this;
  }

  /**
   * @brief Shift the patch by the opposite of given indices.
   */
  KOKKOS_INLINE_FUNCTION Patch& ishift(auto... is)
  {
    m_domain.subtract(is...);
    return *this;
  }

private:

  Parent m_parent; ///< The parent
  Domain m_domain; ///< The domain
};

template <typename T>
concept AnyPatch = is_specialization<Patch, T>;

/**
 * @relatesalso Patch
 * @brief Get the root data container.
 */
KOKKOS_INLINE_FUNCTION const auto& root(const AnyPatch auto& patch)
{
  return root(patch.parent());
}

/**
 * @relatesalso Patch
 * @brief Identity for compatibility with `Patch`.
 */
KOKKOS_INLINE_FUNCTION const auto& root(const AnyImage auto& image)
{
  return image;
}

/**
 * @relatesalso Image
 * @relatesalso Patch
 * @brief Make a patch of an image.
 * 
 * @param in The input container
 * @param domain The patch domain
 * 
 * If the domain is larger than the image domain, then their intersection is used.
 * 
 * @see slice()
 */
template <typename T, int N, typename TContainer, typename U, SliceType... TSlices>
auto where(const Image<T, N, TContainer>& in, const Slice<U, TSlices...>& domain)
{
  return where(in, box(domain & in.domain()));
}

/**
 * @copydoc where()
 */
template <typename T, int N, typename TContainer, typename U>
auto where(const Image<T, N, TContainer>& in, const GBox<U, N>& domain)
{
  return Patch<Image<T, N, TContainer>, GBox<U, N>>(in, domain & in.domain());
}

/**
 * @copydoc where()
 */
template <typename TParent, typename TDomain, typename U>
auto where(const Patch<TParent, TDomain>& in, const GBox<U, TParent::n>& domain)
{
  return Patch<TParent, TDomain>(root(in), domain & in.domain());
}

/**
 * @brief Copy a patch to a given memory space if not already accessible from that space.
 */
template <typename TSpace, typename TParent, typename TDomain>
decltype(auto) on_device(const Patch<TParent, TDomain>& in)
{
  return Patch(on_device<TSpace>(in.parent()), in.domain());
}

/**
 * @brief Copy a patch to host if not already accessible from that space.
 */
template <typename TParent, typename TDomain>
decltype(auto) on_host(const Patch<TParent, TDomain>& in)
{
  return on_device<Kokkos::HostSpace>(in);
}

// FIXME Mask-based patch
// FIXME Sequence/Path-based patch

} // namespace Linx

#endif
