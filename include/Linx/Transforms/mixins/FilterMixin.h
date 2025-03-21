// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_FILTERMIXIN_H
#define LINX_TRANSFORMS_FILTERMIXIN_H

#include "Linx/Data/Sequence.h"

#include <string>

namespace Linx {

template <typename T>
class OffsetBasedPatch {
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
  KOKKOS_INLINE_FUNCTION explicit OffsetBasedPatch(T* data, const auto& offsets) :
      m_data(data),
      m_begin(offsets.data()),
      m_end(m_begin + offsets.size()),
      m_it(m_begin)
  {}

  KOKKOS_INLINE_FUNCTION OffsetBasedPatch(const OffsetBasedPatch& rhs) :
      m_data(rhs.m_data),
      m_begin(rhs.m_begin),
      m_end(rhs.m_end),
      m_it(rhs.m_it)
  {}

  KOKKOS_INLINE_FUNCTION OffsetBasedPatch& operator=(const OffsetBasedPatch& rhs)
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

  KOKKOS_INLINE_FUNCTION OffsetBasedPatch begin() const
  {
    auto out = *this;
    out.m_it = m_begin;
    return out;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedPatch end() const
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

  KOKKOS_INLINE_FUNCTION OffsetBasedPatch& operator++()
  {
    ++m_it;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedPatch operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedPatch& operator+=(int i)
  {
    m_it += i;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedPatch operator+(int i) const
  {
    auto out = *this;
    out.m_it += i;
    return out;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedPatch& operator--()
  {
    --m_it;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedPatch operator--(int)
  {
    auto out = *this;
    --(*this);
    return out;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedPatch& operator-=(int i)
  {
    m_it -= i;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedPatch operator-(int i) const
  {
    auto out = *this;
    out.m_it -= i;
    return out;
  }

  KOKKOS_INLINE_FUNCTION difference_type operator-(const OffsetBasedPatch& rhs) const
  {
    return m_it - rhs.m_it;
  }

  KOKKOS_INLINE_FUNCTION bool operator==(const OffsetBasedPatch& rhs) const
  {
    return m_it == rhs.m_it;
  }

  KOKKOS_INLINE_FUNCTION bool operator!=(const OffsetBasedPatch& rhs) const
  {
    return m_it != rhs.m_it;
  }

private:

  T* m_data; ///< The reference data
  const std::ptrdiff_t* m_begin; ///< The begin offset iterator
  const std::ptrdiff_t* m_end; ///< The end offset iterator
  const std::ptrdiff_t* m_it; ///< The current offset iterator
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
 * using Laplacian<0, 1> = Laplacian<0> + Laplacian<1>;
 * \endcode
 * 
 * Possible usage:
 * 
 * \code
 * auto cropped = Linx::Convolve(kernel)(in);
 * auto [cropped] = P::Run("convolution") | in | Linx::Convolve(kernel);
 * auto extrapolated = Linx::Convolve(kernel).pad(0)(in);
 * auto extrapolated = Linx::Convolve(kernel)(Linx::Pad(0)(in));
 * auto extrapolated = Linx::Convolve(kernel)(Linx::NearestNeighborExtrapolation()(in));
 * auto extrapolated = Linx::Convolve(kernel)(Linx::NearestNeighborExtrapolation()(in));
 * auto [extrapolated] = P::Run("convolution") | in | Linx::Pad(0) | Linx::Convolve(kernel);
 * auto [extrapolated] = P::Run("convolution") | in | Linx::Convolve(kernel).pad(0);
 * auto [extrapolated] = P::Run("convolution") | in | Linx::NearestNeighborExtrapolation() | Linx::Convolve(kernel);
 * 
 * auto cropped = Linx::Erode(radius)(in);
 * auto [cropped] = P::Run("erosion") | in | Linx::Erode(radius);
 * auto extrapolated = Linx::Erode(radius)(Linx::Pad()(in)); // Deduce padding
 * auto [extrapolated] = P::Run("erosion") | in | Linx::WithExtrapolation() | Linx::Erode(radius);
 * 
 * auto cropped = Linx::Erode(radius) * Linx::Dilate(radius) * in;
 * auto [cropped] = P::Run("opening") | in | Linx::Erode(radius) * Linx::Dilate(radius);
 * auto extrapolated = Linx::Erode(radius) * Linx::Dilate(radius) * Linx::Pad() * in;
 * auto [extrapolated] = P::Run("opening") | in | Linx::Pad() | Linx::Erode(radius) * Linx::Dilate(radius);
 * auto [extrapolated] = P::Run("opening") | in | Linx::Pad(Linx::Erode(radius) * Linx::Dilate(radius));
 * auto [extrapolated] = P::Run("opening") | in | Linx::Erode(radius).pad() * Linx::Dilate(radius).pad();
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
    using Out = Image<T, TIn::n>; // FIXME Rebind<TIn>::As<T>
    Out out(compose_label(LINX_CRTP_CONST_DERIVED.label(), in), in.shape());
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
    for_each<Kokkos::Serial>("compute_offsets()", footprint(), [&](std::integral auto... is) {
      *it = m_in.offset(is...);
      ++it;
    });
    Kokkos::deep_copy(m_offsets.container(), offsets_on_host.container());
  }

  decltype(auto) footprint() const
  {
    return m_filter.footprint();
  }

  auto domain() const
  {
    return Box(m_in.domain().start() - footprint().start(), m_in.domain().stop() - footprint().stop() + 1);
  }

  KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... is) const
  {
    return LINX_CRTP_CONST_DERIVED.reduce(OffsetBasedPatch(&this->m_in(is...), m_offsets)); // FIXME pool of patches?
  }

  template <typename TOut>
  void copy_to(TOut& out) const
  {
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

  using value_type = typename TKernel::value_type;

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
      *oit = m_in.offset(is...);
      *wit = kernel_on_host(is...);
      ++oit;
      ++wit;
    });
    Kokkos::deep_copy(m_offsets.container(), offsets_on_host.container());
    Kokkos::deep_copy(m_weights.container(), weights_on_host.container());
  }

  auto footprint() const
  {
    return m_filter.footprint();
  }

  auto domain() const
  {
    return Box(m_in.domain().start() - footprint().start(), m_in.domain().stop() - footprint().stop() + 1);
  }

  KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... is) const
  {
    return LINX_CRTP_CONST_DERIVED.reduce(OffsetBasedPatch(&m_in(is...), m_offsets));
  }

  template <typename TOut>
  void copy_to(TOut& out) const
  {
    for_each<execution_space>("copy_to", domain(), Copy(LINX_CRTP_CONST_DERIVED, out));
  }

protected:

  TFilter m_filter; ///< The filter
  Sequence<std::ptrdiff_t, -1> m_offsets; ///< The footprint offsets in the input
  Sequence<value_type, -1> m_weights; ///< The weights in the same order
  decltype(as_readonly(std::declval<TIn>())) m_in; ///< The input
};

} // namespace Linx

#endif
