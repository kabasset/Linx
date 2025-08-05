// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_MIXINS_FILTER_H
#define LINX_TRANSFORMS_MIXINS_FILTER_H

#include "Linx/Base/mixins/Strided.h"
#include "Linx/Data/Patch.h"
#include "Linx/Data/Profile.h"
#include "Linx/Transforms/Resampling.h" // Pad FIXME rm when all extrapolations are supported

#include <string>

namespace Linx {

/**
 * @brief Filter with extrapolation.
 */
template <typename TParent, typename TMethod>
class ExtrapolatedFilter { // FIXME make the default, force nullprt for no extrapolation

public:

  using Parent = TParent; ///< The parent filter
  using Method = TMethod; ///< The extrapolation method

  /**
   * @brief Constructor.
   */
  ExtrapolatedFilter(Parent parent, Method method) : m_parent(parent), m_method(LINX_MOVE(method)) {}

  /**
   * @brief Filter label.
   */
  std::string label() const // FIXME rm
  {
    return compose_label("extrapolate", m_parent, m_method);
  }

  /**
   * @brief Parent filter.
   */
  KOKKOS_INLINE_FUNCTION const Parent& parent() const // FIXME rm
  {
    return m_parent;
  }

  /**
   * @brief Extrapolation method.
   */
  KOKKOS_INLINE_FUNCTION const Method& method() const
  {
    return m_method;
  }

  /**
   * @brief Lazy evaluator.
   * 
   * No computation is performed immediately.
   * The returned object can then evaluate the filter at chosen positions.
   * This is especially useful to filter only a few points of the input.
   */
  template <typename TIn>
  auto lazy(const TIn& in) const
  {
    if constexpr (std::is_same_v<Method, std::nullptr_t>) {
      // FIXME return typename TDerived::Lazy<TIn>(LINX_CRTP_CONST_DERIVED, in);
    } else {
      auto domain = dilate(bbox(in.domain()), m_parent.footprint());
      auto extrapolated = Linx::no_init<typename TIn::element_type>("extrapolated", domain);
      extrapolated.copy_from(Extrapolation(in, m_method));
      return m_parent.lazy(extrapolated);
    }
  }

  /**
   * @brief Apply the filter on a full input.
   */
  template <typename TIn>
  auto operator()(const TIn& in) const
  {
    using T = std::remove_cvref_t<typename TParent::Lazy<TIn>::value_type>;
    auto out = same_layout<T>(compose_label(m_parent.label(), in), in);
    // TODO if Method == Copy, out = Patch(+in, inner_domain);
    transform(in, out);
    // TODO optimize memory usage by replacing lazy(in) with:
    // - for (box : box_difference(domain, footprint)) extrapolate
    // - for inner don't extrapolate
    return out;
  }

  /**
   * @brief Apply the filter into a preallocated output.
   * 
   * The domain of the output is used as the evaluation domain.
   */
  template <typename TIn, typename TOut>
  void transform(const TIn& in, const TOut& out) const
  {
    out.copy_from(lazy(in));
  }

private:

  Parent m_parent;
  Method m_method;
};

/**
 * @brief Filtering task mixin.
 */
template <typename TDerived>
class FilterMixin {
public:

  /**
   * @brief Lazy evaluator.
   * 
   * No computation is performed immediately.
   * The returned object can then evaluate the filter at chosen positions.
   * This is especially useful to filter only a few points of the input.
   */
  template <typename TIn>
  auto lazy(const TIn& in) const
  {
    return typename TDerived::Lazy<TIn>(LINX_CRTP_CONST_DERIVED, in);
  }

  /**
   * @brief Filter an image without extrapolation.
   * 
   * The output image has the same shape as the input image.
   * The borders of the output image are default-initialized.
   */
  template <typename TIn>
  auto operator()(const TIn& in) const
  {
    using T = std::remove_cvref_t<typename TDerived::Lazy<TIn>::value_type>;
    auto domain = erode(bbox(in.domain()), LINX_CRTP_CONST_DERIVED.footprint());
    auto out = same_layout<T>(compose_label(LINX_CRTP_CONST_DERIVED.label(), in), in);
    transform(in, Patch(Forward(), out, domain));
    return out;
  }

  /**
   * @brief Filter an image without extrapolation.
   * 
   * All the positions of the output domain are evaluated.
   */
  template <typename TIn, typename TOut>
  void transform(const TIn& in, const TOut& out) const
  {
    out.copy_from(lazy(in));
  }

  /**
   * @brief Use extrapolation with a given value or method.
   */
  auto pad(const auto& value) const // FIXME rename as padded()? extrapolated?
  {
    return ExtrapolatedFilter(
        LINX_CRTP_CONST_DERIVED,
        Pad(value)); // FIXME removing Pad() should work to accept any method
  }
};

/**
 * @brief Footprint-defined filter mixin.
 */
template <typename TFootprint, typename TDerived>
class SpatialFilterMixin : public FilterMixin<TDerived> {
public:

  /**
   * @brief Constructor.
   */
  SpatialFilterMixin(TFootprint footprint) : m_footprint(LINX_MOVE(footprint)) {}

  /**
   * @brief Spatial footprint.
   */
  const auto& footprint() const
  {
    return m_footprint;
  }

private:

  TFootprint m_footprint; ///< The footprint
};

/**
 * @brief The helper class returned by `SpatialFilterMixin::lazy()`.
 */
template <typename TFilter, typename TIn, typename TDerived>
class LazySpatialFilterMixin {
public:

  using execution_space = typename TIn::execution_space;

  LazySpatialFilterMixin(TFilter filter, const TIn& in) :
      m_filter(LINX_MOVE(filter)),
      m_neighbors(try_as_readonly(in), m_filter.footprint())
  {}

  std::string label() const
  {
    return m_filter.label(); // FIXME should be optional
  }

  KOKKOS_INLINE_FUNCTION decltype(auto) footprint() const
  {
    return m_filter.footprint();
  }

  KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... is) const
  {
    return LINX_CRTP_CONST_DERIVED.reduce(m_neighbors.shifted_span(is...));
  }

protected:

  using Neighbors = decltype(Profile(try_as_readonly(std::declval<TIn>()), 0));

  TFilter m_filter; ///< The filter
  Neighbors m_neighbors; ///< The profile of the input
};

/**
 * @brief Kernel-defined filter mixin.
 */
template <typename TKernel, typename TDerived>
class WeightedFilterMixin : public FilterMixin<TDerived> {
public:

  using value_type = const typename TKernel::value_type;

  WeightedFilterMixin(const TKernel& kernel) : m_kernel(try_as_readonly(kernel)) {}

  decltype(auto) footprint() const
  {
    return m_kernel.domain();
  }

  KOKKOS_INLINE_FUNCTION const auto& kernel() const
  {
    return m_kernel;
  }

  KOKKOS_INLINE_FUNCTION const auto& kernel(std::integral auto... position) const
  {
    return m_kernel(position...);
  }

  /**
   * @brief Divide the kernel values by the kernel sum.
   */
  TDerived& normalize()
  {
    m_kernel /= sum(m_kernel);
    return LINX_CRTP_CONST_DERIVED;
  }

private:

  decltype(try_as_readonly(std::declval<TKernel>())) m_kernel; ///< The kernel
};

namespace Impl {

template <typename TFilter, typename TNeighbors, typename TWeights>
struct EmplaceWeights {
  TFilter m_filter;
  TNeighbors m_neighbors;
  TWeights m_weights;

  KOKKOS_INLINE_FUNCTION void operator()(std::integral auto... position) const
  {
    const auto& w = m_filter.kernel(position...);
    // TODO discard if w == identity_element()'s
    auto index = m_neighbors.emplace_back(position...);
    m_weights[index] = w;
  }
};

} // namespace Impl

/**
 * @brief The helper class returned by `WeightedFilterMixin::lazy()`.
 */
template <typename TFilter, typename TIn, typename TDerived>
class LazyWeightedFilterMixin {
public:

  using value_type = typename TFilter::value_type;
  using element_type = std::remove_cvref_t<value_type>;
  using execution_space = typename TIn::execution_space;

  LazyWeightedFilterMixin(TFilter filter, const TIn& in) :
      m_filter(LINX_MOVE(filter)),
      m_neighbors(try_as_readonly(in), m_filter.footprint().size()),
      m_weights("weights", m_neighbors.capacity())
  {
    init_impl();
  }

  void init_impl() const
  {
    for_each<execution_space>(
        "compute offsets",
        m_filter.footprint(),
        Impl::EmplaceWeights {m_filter, m_neighbors, m_weights});
    // TODO resize m_weights if discarded identity_element()
  }

  /**
   * @brief Filter label.
   */
  std::string label() const
  {
    return m_filter.label(); // FIXME should be optional
  }

  /**
   * @brief Filter footprint.
   */
  KOKKOS_INLINE_FUNCTION decltype(auto) footprint() const
  {
    return m_filter.footprint();
  }

  /**
   * @brief Single-position evaluation.
   */
  KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... position) const
  {
    return LINX_CRTP_CONST_DERIVED.reduce(m_neighbors.shifted_span(position...));
  }

protected:

  using Neighbors = decltype(Profile(try_as_readonly(std::declval<TIn>()), 0));
  using Weights = decltype(no_init<element_type, execution_space>("", 0)); // FIXME adapt domain, if possible static

  TFilter m_filter; ///< The filter
  Neighbors m_neighbors; ///< The image profile along the footprint
  Weights m_weights; ///< The weights in the same order
};

} // namespace Linx

#endif
