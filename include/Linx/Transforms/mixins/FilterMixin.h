// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_TRANSFORMS_FILTERMIXIN_H
#define LINX_TRANSFORMS_FILTERMIXIN_H

#include "Linx/Base/ArrayPool.h"
#include "Linx/Data/Sequence.h"

#include <string>

namespace Linx {

template <typename T>
class OffsetBasedIterator {
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
   * @param index The current index
   * @param args Arguments forwarded to the offsets sequence constructor
   */
  KOKKOS_INLINE_FUNCTION explicit OffsetBasedIterator(T* data, std::size_t index, const auto& offsets) :
      m_data(data),
      m_index(index),
      m_offsets(offsets)
  {}

  KOKKOS_INLINE_FUNCTION static OffsetBasedIterator begin(T* data, const auto& offsets)
  {
    return OffsetBasedIterator(data, 0, offsets);
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedIterator end() const
  {
    return OffsetBasedIterator(m_data, m_offsets.size(), m_offsets);
  }

  KOKKOS_INLINE_FUNCTION reference operator[](int i) const
  {
    return m_data[m_offsets[i]];
  }

  KOKKOS_INLINE_FUNCTION reference operator*() const
  {
    return m_data[m_offsets[m_index]];
  }

  KOKKOS_INLINE_FUNCTION pointer operator->() const
  {
    return m_data + m_offsets[m_index];
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedIterator& operator++()
  {
    ++m_index;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedIterator operator++(int)
  {
    auto out = *this;
    ++(*this);
    return out;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedIterator& operator+=(int i)
  {
    m_index += i;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedIterator operator+(int i) const
  {
    auto out = *this;
    out.m_index += i;
    return out;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedIterator& operator--()
  {
    --m_index;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedIterator operator--(int)
  {
    auto out = *this;
    --(*this);
    return out;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedIterator& operator-=(int i)
  {
    m_index -= i;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION OffsetBasedIterator operator-(int i) const
  {
    auto out = *this;
    out.m_index -= i;
    return out;
  }

  KOKKOS_INLINE_FUNCTION difference_type operator-(const OffsetBasedIterator& rhs) const
  {
    return m_index - rhs.m_index;
  }

  KOKKOS_INLINE_FUNCTION bool operator==(const OffsetBasedIterator& rhs) const
  {
    return m_index == rhs.m_index;
  }

  KOKKOS_INLINE_FUNCTION bool operator!=(const OffsetBasedIterator& rhs) const
  {
    return m_index != rhs.m_index;
  }

private:

  T* m_data;
  std::size_t m_index; // FIXME iterator on offsets?
  const Sequence<std::ptrdiff_t, -1>& m_offsets; // FIXME -1 by default
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
template <typename TFootprint, typename TDerived>
class SpatialFilterMixin {
public:

  SpatialFilterMixin(TFootprint footprint) : m_footprint(LINX_MOVE(footprint)) {}

  template <typename TIn>
  auto operator()(const TIn& in) const
  {
    return typename TDerived::Apply<TIn>(m_footprint, in);
  }

  const auto& footprint() const
  {
    return m_footprint;
  }

private:

  TFootprint m_footprint; ///< The footprint
};

template <typename TFootprint, typename TIn, typename TDerived>
class ApplySpatialFilterMixin {
public:

  ApplySpatialFilterMixin(TFootprint footprint, const TIn& in) :
      m_footprint(LINX_MOVE(footprint)),
      m_offsets("offsets", m_footprint.size()),
      m_in(as_readonly(in))
  {
    auto offsets_on_host = on_host(m_offsets);
    auto index = std::make_shared<Index>(0); // Required for copying into for_each
    auto front = &m_in.front();
    for_each<Kokkos::Serial>(
        "compute_offsets()", // FIXME analytic through strides?
        m_footprint,
        KOKKOS_LAMBDA(std::integral auto... is) {
          offsets_on_host[*index] = &m_in(is...) - front;
          ++(*index);
        });
    Linx::copy_to(offsets_on_host, m_offsets); // FIXME offsets_on_host.copy_to(m_offsets)
  }

  KOKKOS_INLINE_FUNCTION const auto& footprint() const
  {
    return m_footprint;
  }

  auto domain() const
  {
    return Box(m_in.domain().start() - m_footprint.start(), m_in.domain().stop() - m_footprint.stop() + 1);
  }

  KOKKOS_INLINE_FUNCTION auto operator()(std::integral auto... is) const
  {
    const typename TIn::value_type* data = &this->m_in(is...);
    auto begin = OffsetBasedIterator<const typename TIn::value_type>(data, 0, m_offsets);
    auto end = begin.end();
    return LINX_CRTP_CONST_DERIVED.reduce(LINX_MOVE(begin), LINX_MOVE(end));
  }

  void copy_to(const auto& out) const
  {
    for_each(
        "copy_to",
        domain(),
        KOKKOS_LAMBDA(auto... is) { out(is...) = (*this)(is...); }); // FIXME make generic
  }

protected:

  TFootprint m_footprint; ///< The footprint
  Sequence<std::ptrdiff_t, -1> m_offsets; ///< The footprint offsets in the input
  decltype(as_readonly(std::declval<TIn>())) m_in; ///< The input
};

template <typename TFootprint>
class SumFilter : public SpatialFilterMixin<TFootprint, SumFilter<TFootprint>> {
public:

  SumFilter(TFootprint footprint) : SpatialFilterMixin<TFootprint, SumFilter>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "SumFilter";
  }

  template <typename TIn>
  struct Apply : public ApplySpatialFilterMixin<TFootprint, TIn, Apply<TIn>> {
    using ApplySpatialFilterMixin<TFootprint, TIn, Apply>::ApplySpatialFilterMixin;

    KOKKOS_INLINE_FUNCTION auto reduce(auto begin, auto end) const
    {
      return std::reduce(LINX_MOVE(begin), LINX_MOVE(end));
    }
  };
};

template <typename TIn, typename TDerived>
class MorphologyFilterMixin { // FIXME simply FilterMixin?
public:

  MorphologyFilterMixin(const auto& strel, const TIn& in) :
      MorphologyFilterMixin(Sequence<std::ptrdiff_t, -1>("offsets", strel.size()), in)
  {
    auto offsets_on_host = on_host(m_offsets);
    auto index = std::make_shared<Index>(0);
    auto front = &m_in.front();
    for_each<Kokkos::Serial>(
        "compute_offsets()", // FIXME analytic through strides?
        strel,
        KOKKOS_LAMBDA(std::integral auto... is) {
          offsets_on_host[*index] = &m_in(is...) - front;
          ++(*index);
        });
    copy_to(offsets_on_host, m_offsets); // FIXME offsets_on_host.copy_to(m_offsets)
  }

protected:

  MorphologyFilterMixin(const Sequence<std::ptrdiff_t, -1>& offsets, const TIn& in) :
      m_offsets(offsets),
      m_in(as_readonly(in))
  {}

  Sequence<std::ptrdiff_t, -1> m_offsets;
  decltype(as_readonly(std::declval<TIn>())) m_in;
};

template <typename TIn, typename TDerived>
const TDerived& as_readonly(const MorphologyFilterMixin<TIn, TDerived>& in)
{
  return static_cast<const TDerived&>(in);
}

template <typename TKernel, typename TIn, typename TDerived>
class WeightedFilterMixin : public MorphologyFilterMixin<TIn, TDerived> {
public:

  WeightedFilterMixin(const auto& kernel, const auto& in) :
      MorphologyFilterMixin<TIn, TDerived>(Sequence<std::ptrdiff_t, -1>("offsets", kernel.size()), in),
      m_weights("weights", kernel.size())
  {
    // FIXME delegate m_offsets computation to Morphology?

    auto offsets_on_host = on_host(this->m_offsets);
    auto weights_on_host = on_host(m_weights);
    auto index = std::make_shared<Index>(0);
    auto front = &this->m_in.front();
    for_each<Kokkos::Serial>(
        "compute_offsets()",
        kernel.domain(),
        KOKKOS_LAMBDA(std::integral auto... is) {
          offsets_on_host[*index] = &this->m_in(is...) - front;
          weights_on_host[*index] = kernel(is...);
          ++(*index);
        });
    copy_to(offsets_on_host, this->m_offsets); // FIXME offsets_on_host.copy_to(m_offsets)
    copy_to(weights_on_host, m_weights); // FIXME
  }

protected:

  Sequence<typename TKernel::element_type, -1> m_weights;
};

} // namespace Linx

#endif
