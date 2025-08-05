// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_PATCH_H
#define LINX_DATA_PATCH_H

#include "Linx/Base/Functional.h"
#include "Linx/Base/Types.h"
#include "Linx/Data/Image.h"

#include <Kokkos_Core.hpp>

namespace Linx {

/**
 * @ingroup regions
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
   * 
   * The patch parent will be `root(in)` and its domain `region & in.domain()`.
   * This ensures that the patch refers directly to the root data container,
   * and that the patch domain is completely inside the root data container domain.
   * 
   * There is another, `Forward`-tagged constructor, which bypasses those transforms.
   */
  Patch(const auto& in, const auto& region) : m_parent(root(in)), m_domain(region & in.domain()) {}

  /**
   * @brief Forwarding constructor.
   * 
   * The parent and domain are unaltered, which may result in a patch of patch,
   * and a domain which spans outside of the parent domain.
   */
  Patch(Forward, const Parent& parent, Domain domain) : m_parent(parent), m_domain(LINX_MOVE(domain)) {}

  /**
   * @brief Parent.
   */
  KOKKOS_INLINE_FUNCTION const Parent& parent() const
  {
    return m_parent;
  }

  /**
   * @brief Domain.
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
   * @brief Domain size.
   */
  KOKKOS_INLINE_FUNCTION auto size() const
  {
    return domain().size();
  }

  /**
   * @brief Parent stride along given axis.
   */
  KOKKOS_INLINE_FUNCTION auto stride(std::integral auto i) const
  {
    return m_parent.stride(i);
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
   * @brief Reference to the element at a given domain-local position.
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
KOKKOS_INLINE_FUNCTION const auto& root(const auto& in)
{
  return in;
}

namespace Impl {

template <typename TIn, typename TDomain>
struct PatchTraits {
  using Parent = std::remove_cvref_t<decltype(root(std::declval<TIn>()))>;
  using Domain = std::remove_cvref_t<decltype(std::declval<TDomain>() & std::declval<TIn>().domain())>;
  using Type = Patch<Parent, Domain>;
};

} // namespace Impl

template <typename TIn, typename TDomain>
Patch(const TIn&, const TDomain&)
    -> Patch<typename Impl::PatchTraits<TIn, TDomain>::Parent, typename Impl::PatchTraits<TIn, TDomain>::Domain>;

template <typename TParent, typename TDomain>
decltype(auto) as_readonly(const Patch<TParent, TDomain>& in)
{
  return Patch(Forward(), try_as_readonly(in.parent()), in.domain());
}

/**
 * @brief Copy a patch to a given memory space if not already accessible from that space.
 */
template <typename TSpace, typename TParent, typename TDomain>
decltype(auto) on_device(const Patch<TParent, TDomain>& in)
{
  return Patch(Forward(), on_device<TSpace>(in.parent()), in.domain());
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

} // namespace Linx

#endif
