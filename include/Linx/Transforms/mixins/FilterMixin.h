// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_FILTERMIXIN_H
#define LINX_TRANSFORMS_FILTERMIXIN_H

#include "Linx/Base/mixins/Strided.h"
#include "Linx/Data/Patch.h"
#include "Linx/Data/Profile.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Transforms/Resampling.h" // FIXME used?
#include "Linx/Transforms/Shift.h"

#include <string>

namespace Linx {

template <typename T>
class OffsetBasedRange {
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
  KOKKOS_INLINE_FUNCTION explicit OffsetBasedRange(T* data, const auto& offsets) :
      m_data(data),
      m_begin(offsets.data()),
      m_end(m_begin + offsets.size()),
      m_it(m_begin)
  {}

  KOKKOS_INLINE_FUNCTION OffsetBasedRange(const OffsetBasedRange& rhs) :
      m_data(rhs.m_data),
      m_begin(rhs.m_begin),
      m_end(rhs.m_end),
      m_it(rhs.m_it)
  {}

  KOKKOS_INLINE_FUNCTION OffsetBasedRange& operator=(const OffsetBasedRange& rhs)
  {
    m_data = rhs.m_data;
    m_begin = rhs.m_begin;
    m_end = rhs.m_end;
    m_it = rhs.m_it;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION void reset(T* data)
  {
    m_data = data;
    m_it = m_begin;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedRange begin() const
  {
    auto out = *this;
    out.m_it = m_begin;
    return out;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedRange end() const
  {
    auto out = *this;
    out.m_it = m_end;
    return out;
  }

  KOKKOS_INLINE_FUNCTION auto ssize() const
  {
    return m_end - m_begin;
  }

  KOKKOS_INLINE_FUNCTION auto size() const
  {
    return static_cast<std::size_t>(ssize());
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

  KOKKOS_INLINE_FUNCTION OffsetBasedRange& operator++()
  {
    ++m_it;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedRange operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedRange& operator+=(int i)
  {
    m_it += i;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedRange operator+(int i) const
  {
    auto out = *this;
    out.m_it += i;
    return out;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedRange& operator--()
  {
    --m_it;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedRange operator--(int)
  {
    auto out = *this;
    --(*this);
    return out;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedRange& operator-=(int i)
  {
    m_it -= i;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedRange operator-(int i) const
  {
    auto out = *this;
    out.m_it -= i;
    return out;
  }

  KOKKOS_INLINE_FUNCTION difference_type operator-(const OffsetBasedRange& rhs) const
  {
    return m_it - rhs.m_it;
  }

  KOKKOS_INLINE_FUNCTION bool operator==(const OffsetBasedRange& rhs) const
  {
    return m_it == rhs.m_it;
  }

  KOKKOS_INLINE_FUNCTION bool operator!=(const OffsetBasedRange& rhs) const
  {
    return m_it != rhs.m_it;
  }

private:

  T* m_data; ///< The reference data
  const std::ptrdiff_t* m_begin; ///< The begin offset iterator
  const std::ptrdiff_t* m_end; ///< The end offset iterator
  const std::ptrdiff_t* m_it; ///< The current offset iterator
};

template <typename TParent, typename TMethod>
class ExtrapolatedFilter {
public:

  using Parent = TParent;
  using Method = TMethod;

  ExtrapolatedFilter(Parent parent, Method method) : m_parent(parent), m_method(LINX_MOVE(method)) {}

  std::string label() const
  {
    return compose_label("extrapolate", m_parent, m_method);
  }

  KOKKOS_INLINE_FUNCTION const Parent& parent() const
  {
    return m_parent;
  }

  KOKKOS_INLINE_FUNCTION const Method& method() const
  {
    return m_method;
  }

  template <typename TIn>
  auto lazy(const TIn& in) const
  {
    auto domain = bbox(in.domain()) + m_parent.footprint(); // FIXME test
    auto extrapolated = Shift(TIn("extrapolated", domain.shape()), domain.start());
    extrapolated.copy_from(Extrapolation(in, m_method)); // TODO optimize
    return m_parent.lazy(Patch(Forward(), extrapolated, in.domain()));
    // FIXME return m_parent.lazy(extrapolated): no domain() in Lazy
  }

  template <typename TIn>
  auto operator()(const TIn& in) const
  {
    using T = std::remove_cvref_t<typename TParent::Lazy<TIn>::value_type>;
    auto out = same_layout<T>(compose_label(m_parent.label(), in), in);
    transform(in, out);
    return out;
  }

  template <typename TIn, typename TOut>
  void transform(const TIn& in, const TOut& out) const
  {
    out.copy_from(lazy(in)); // Use out's domain instead of in's
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
    auto out = same_layout<T>(compose_label(LINX_CRTP_CONST_DERIVED.label(), in), in);
    transform(in, out);
    return out;
  }

  /**
   * @brief Filter an image without extrapolation.
   * 
   * All the positions of the filtering domain are evaluated.
   */
  template <typename TIn, typename TOut>
  void transform(const TIn& in, const TOut& out) const
  {
    lazy(in).copy_to(out);
  }

  auto pad(const auto& value) const
  {
    return ExtrapolatedFilter(LINX_CRTP_CONST_DERIVED, Pad(value));
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
private:

  using Profile = decltype(Profile(try_as_readonly(std::declval<TIn>()), 0));

public:

  using execution_space = typename TIn::execution_space;

  LazySpatialFilterMixin(TFilter filter, const TIn& in) :
      m_filter(LINX_MOVE(filter)),
      m_profile(try_as_readonly(in), m_filter.footprint())
  {}

  std::string label() const
  {
    return m_filter.label();
  }

  decltype(auto) footprint() const
  {
    return m_filter.footprint();
  }

  auto domain() const
  {
    auto in_box = bbox(m_profile.parent().domain());
    auto footprint_box = bbox(footprint());
    return Box(
        in_box.start() - resize<TIn::n, Kokkos::HostSpace>("start", footprint_box.start()),
        in_box.stop() - resize<TIn::n, Kokkos::HostSpace>("stop - 1", footprint_box.stop() - 1)); // TODO support -1
  }

  KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... is) const
  {
    return LINX_CRTP_CONST_DERIVED.reduce(m_profile.shifted_span(is...));
  }

  template <typename TOut>
  void copy_to(TOut& out) const
  {
    static_assert(Kokkos::SpaceAccessibility<execution_space, typename TOut::memory_space>::accessible);
    for_each<execution_space>("copy_to", domain(), Copy(LINX_CRTP_CONST_DERIVED, out));
  }

protected:

  TFilter m_filter; ///< The filter
  Profile m_profile; ///< The profile of the input
};

/**
 * @brief Kernel-defined filter mixin.
 */
template <typename TKernel, typename TDerived>
class WeightedFilterMixin : public FilterMixin<TDerived> {
public:

  using value_type = const typename TKernel::value_type;

  WeightedFilterMixin(TKernel kernel) : m_kernel(LINX_MOVE(kernel)) {}

  auto footprint() const
  {
    return m_kernel.domain();
  }

  const auto& kernel() const
  {
    return m_kernel;
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

  TKernel m_kernel; ///< The kernel
};

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
      m_offsets("offsets", m_filter.footprint().size()),
      m_weights("weights", m_offsets.size()),
      m_in(try_as_readonly(in))
  {
    const auto& offsets_on_host = on_host(m_offsets);
    const auto& weights_on_host = on_host(m_weights);
    const auto& kernel_on_host = on_host(m_filter.kernel());
    auto oit = offsets_on_host.begin();
    auto wit = weights_on_host.begin();
    for_each<Kokkos::Serial>("compute offsets", m_filter.footprint(), [&](std::integral auto... is) {
      *oit = offset_from_origin(m_in, is...);
      *wit = kernel_on_host(is...);
      ++oit;
      ++wit;
    });
    Kokkos::deep_copy(m_offsets.container(), offsets_on_host.container());
    Kokkos::deep_copy(m_weights.container(), weights_on_host.container());
  }

  std::string label() const
  {
    return m_filter.label();
  }

  auto footprint() const
  {
    return m_filter.footprint();
  }

  auto domain() const // FIXME to FilterMixin::operator()
  {
    auto in_box = bbox(m_in.domain());
    auto footprint_box = bbox(footprint());
    return Box(
        in_box.start() - resize<TIn::n>("start", footprint_box.start()),
        in_box.stop() - resize<TIn::n>("stop - 1", footprint_box.stop() - 1)); // TODO support -1
  }

  KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... is) const
  {
    return LINX_CRTP_CONST_DERIVED.reduce(OffsetBasedRange(&m_in(is...), m_offsets));
  }

  template <typename TOut>
  void copy_to(TOut& out) const
  {
    static_assert(Kokkos::SpaceAccessibility<execution_space, typename TOut::memory_space>::accessible);
    for_each<execution_space>("copy_to", domain(), Copy(LINX_CRTP_CONST_DERIVED, out));
  }

protected:

  template <typename T>
  using Vector = Sequence<T, -1, SequenceContainer<T, -1, execution_space>>;

  TFilter m_filter; ///< The filter
  Vector<std::ptrdiff_t> m_offsets; ///< The footprint offsets in the input
  Vector<element_type> m_weights; ///< The weights in the same order
  decltype(try_as_readonly(std::declval<TIn>())) m_in; ///< The input
};

} // namespace Linx

#endif
