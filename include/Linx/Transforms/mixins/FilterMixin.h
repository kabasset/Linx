// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_FILTERMIXIN_H
#define LINX_TRANSFORMS_FILTERMIXIN_H

#include "Linx/Data/Patch.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Transforms/Resampling.h"
#include "Linx/Transforms/Shift.h"

#include <string>

namespace Linx {

template <typename T>
class DistanceBasedRange {
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
  KOKKOS_INLINE_FUNCTION explicit DistanceBasedRange(T* data, const auto& offsets) :
      m_data(data),
      m_begin(offsets.data()),
      m_end(m_begin + offsets.size()),
      m_it(m_begin)
  {}

  KOKKOS_INLINE_FUNCTION DistanceBasedRange(const DistanceBasedRange& rhs) :
      m_data(rhs.m_data),
      m_begin(rhs.m_begin),
      m_end(rhs.m_end),
      m_it(rhs.m_it)
  {}

  KOKKOS_INLINE_FUNCTION DistanceBasedRange& operator=(const DistanceBasedRange& rhs)
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

  KOKKOS_INLINE_FUNCTION DistanceBasedRange begin() const
  {
    auto out = *this;
    out.m_it = m_begin;
    return out;
  }

  KOKKOS_INLINE_FUNCTION DistanceBasedRange end() const
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

  KOKKOS_INLINE_FUNCTION DistanceBasedRange& operator++()
  {
    ++m_it;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION DistanceBasedRange operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  KOKKOS_INLINE_FUNCTION DistanceBasedRange& operator+=(int i)
  {
    m_it += i;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION DistanceBasedRange operator+(int i) const
  {
    auto out = *this;
    out.m_it += i;
    return out;
  }

  KOKKOS_INLINE_FUNCTION DistanceBasedRange& operator--()
  {
    --m_it;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION DistanceBasedRange operator--(int)
  {
    auto out = *this;
    --(*this);
    return out;
  }

  KOKKOS_INLINE_FUNCTION DistanceBasedRange& operator-=(int i)
  {
    m_it -= i;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION DistanceBasedRange operator-(int i) const
  {
    auto out = *this;
    out.m_it -= i;
    return out;
  }

  KOKKOS_INLINE_FUNCTION difference_type operator-(const DistanceBasedRange& rhs) const
  {
    return m_it - rhs.m_it;
  }

  KOKKOS_INLINE_FUNCTION bool operator==(const DistanceBasedRange& rhs) const
  {
    return m_it == rhs.m_it;
  }

  KOKKOS_INLINE_FUNCTION bool operator!=(const DistanceBasedRange& rhs) const
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
  }

  template <typename TIn>
  auto operator()(const TIn& in) const
  {
    using T = std::remove_cvref_t<typename TParent::Apply<TIn>::value_type>;
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
 * 
 * With:
 * 
 * \code
 * class Convolve : public WeightedFilterMixin;
 * class Erode : public FilterMixin;
 * using Opening = Erode * Dilate
 * \endcode
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
    return typename TDerived::Apply<TIn>(LINX_CRTP_CONST_DERIVED, in);
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
    using T = std::remove_cvref_t<typename TDerived::Apply<TIn>::value_type>;
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
class ApplySpatialFilterMixin {
public:

  // value_type does not necessarily come from TIn
  using execution_space = typename TIn::execution_space;

  ApplySpatialFilterMixin(TFilter filter, const TIn& in) :
      m_filter(LINX_MOVE(filter)),
      m_offsets("offsets", m_filter.footprint().size()),
      m_in(as_readonly(in))
  {
    const auto& offsets_on_host = on_host(m_offsets);
    auto it = offsets_on_host.begin();
    for_each<Kokkos::Serial>("m_offsets", footprint(), [&](std::integral auto... is) {
      *it = m_in.distance_from_origin(is...);
      ++it;
    });
    Kokkos::deep_copy(m_offsets.container(), offsets_on_host.container());
  }

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
    auto in_box = bbox(m_in.domain());
    auto footprint_box = bbox(footprint());
    return Box(
        in_box.start() - pad<TIn::n>(footprint_box.start()),
        in_box.stop() - pad<TIn::n>(footprint_box.stop() - 1));
  }

  KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... is) const
  {
    return LINX_CRTP_CONST_DERIVED.reduce(DistanceBasedRange(&this->m_in(is...), m_offsets)); // FIXME pool of patches?
  }

  template <typename TOut>
  void copy_to(TOut& out) const
  {
    static_assert(Kokkos::SpaceAccessibility<execution_space, typename TOut::memory_space>::accessible);
    for_each<execution_space>("copy_to", domain(), Copy(LINX_CRTP_CONST_DERIVED, out));
  }

protected:

  TFilter m_filter; ///< The filter
  Sequence<std::ptrdiff_t, -1> m_offsets; ///< The footprint offsets in the input
  decltype(as_readonly(std::declval<TIn>())) m_in; ///< The input
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
class ApplyWeightedFilterMixin {
public:

  using value_type = typename TFilter::value_type;
  using element_type = std::remove_cvref_t<value_type>;
  using execution_space = typename TIn::execution_space;

  ApplyWeightedFilterMixin(TFilter filter, const TIn& in) :
      m_filter(LINX_MOVE(filter)),
      m_offsets("offsets", m_filter.footprint().size()),
      m_weights("weights", m_offsets.size()),
      m_in(as_readonly(in))
  {
    const auto& offsets_on_host = on_host(m_offsets);
    const auto& weights_on_host = on_host(m_weights);
    const auto& kernel_on_host = on_host(m_filter.kernel());
    auto oit = offsets_on_host.begin();
    auto wit = weights_on_host.begin();
    for_each<Kokkos::Serial>("compute offsets", m_filter.footprint(), [&](std::integral auto... is) {
      *oit = m_in.distance_from_origin(is...);
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
        in_box.start() - pad<TIn::n>(footprint_box.start()),
        in_box.stop() - pad<TIn::n>(footprint_box.stop() - 1));
  }

  KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... is) const
  {
    return LINX_CRTP_CONST_DERIVED.reduce(DistanceBasedRange(&m_in(is...), m_offsets));
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
  decltype(as_readonly(std::declval<TIn>())) m_in; ///< The input
};

} // namespace Linx

#endif
