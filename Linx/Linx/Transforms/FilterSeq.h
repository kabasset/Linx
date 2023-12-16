// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_FILTERSEQ_H
#define _LINXTRANSFORMS_FILTERSEQ_H

#include "Linx/Base/SeqUtils.h"
#include "Linx/Data/Raster.h"
#include "Linx/Transforms/Extrapolation.h"
#include "Linx/Transforms/impl/Filter.h"

#include <type_traits> // decay

namespace Linx {

/**
 * @brief A sequence of filters.
 */
template <typename... TFilters>
class FilterSeq :
    public FilterMixin<
        typename std::decay_t<decltype((std::declval<TFilters>(), ...))>::Value,
        Box<std::max({TFilters::Dimension...})>,
        FilterSeq<TFilters...>> {
  friend class FilterMixin<
      typename std::decay_t<decltype((std::declval<TFilters>(), ...))>::Value,
      Box<std::max({TFilters::Dimension...})>,
      FilterSeq<TFilters...>>; // FIXME simplify

public:

  /**
   * @brief The last filter value type.
   */
  using Value = typename std::decay_t<decltype((std::declval<TFilters>(), ...))>::Value;

  /**
   * @brief The logical dimension of the composed kernel.
   */
  static constexpr Index Dimension = std::max({TFilters::Dimension...});

  /**
   * @brief Bring to scope FilterMixin::operator*.
   */
  using FilterMixin<
      typename std::decay_t<decltype((std::declval<TFilters>(), ...))>::Value,
      Box<std::max({TFilters::Dimension...})>,
      FilterSeq<TFilters...>>::operator*;

  /**
   * @brief Constructor.
   * 
   * @note Filter sequences are better constructed by multiplying simple filters.
   */
  template <typename... TArgs>
  explicit FilterSeq(TArgs&&... args) : m_filters(std::forward<TArgs>(args)...)
  {}

  /**
   * @brief Combine two sequences of kernels.
   */
  template <typename... UFilters>
  FilterSeq<TFilters..., UFilters...> operator*(const FilterSeq<UFilters...>& rhs) const
  {
    return FilterSeq<TFilters..., UFilters...>(std::tuple_cat(m_filters, rhs.m_filters));
  }

  /**
   * @brief Append a filter to the sequence.
   */
  template <typename T, typename TWindow, typename TDerived>
  auto operator*(FilterMixin<T, TWindow, TDerived>&& rhs) const
  {
    return FilterSeq<TFilters..., FilterMixin<T, TWindow, TDerived>>(
        std::tuple_cat(m_filters, std::forward<FilterMixin<T, TWindow, TDerived>>(rhs)));
  }

  /**
   * @brief Prepend a filter to the sequence.
   */
  template <typename T, typename TWindow, typename TDerived>
  friend auto operator*(FilterMixin<T, TWindow, TDerived>&& lhs, const FilterSeq<TFilters...>& rhs)
  {
    return FilterSeq<FilterMixin<T, TWindow, TDerived>, TFilters...>(
        std::tuple_cat(std::make_tuple(std::forward<FilterMixin<T, TWindow, TDerived>>(lhs)), rhs.m_filters));
  }

  /// @group_properties

  /**
   * @brief Get the i-th filter.
   */
  template <Index I>
  const auto& filter() const
  {
    return std::get<I>(m_filters);
  }

protected:

  /**
   * @brief The logical window of the kernel.
   */
  Box<Dimension> window_impl() const
  {
    // FIXME support Dimension = -1
    auto front = Position<Dimension>::zero();
    auto back = Position<Dimension>::zero();
    seq_foreach(m_filters, [&](const auto& k) {
      const auto& w = box(k.window());
      for (Index i = 0; i < w.dimension(); ++i) {
        // FIXME sum radius instead of min/max
        front[i] = std::min(front[i], w.front()[i]);
        back[i] = std::max(back[i], w.back()[i]);
      }
    });
    return {front, back};
  }

  /**
   * @brief Filter an input raster.
   * 
   * The input is cropped according to the filter window just enough so no extrapolation is required.
   */
  template <typename T, Index N, typename THolder, typename TOut>
  void transform_impl(const Raster<T, N, THolder>& in, TOut& out) const
  {
    const auto outK = upto_kth<sizeof...(TFilters) - 2>(in);
    filter<sizeof...(TFilters) - 1>().transform(outK, out);
  }

  /**
   * @brief Filter an input extrapolated raster.
   * 
   * The input and output must have the same size, although not necessarily the same domain.
   */
  template <typename TRaster, typename TMethod, typename TOut>
  void transform_impl(const Extrapolation<TRaster, TMethod>& in, TOut& out) const
  {
    const auto domain0 = in.domain() + extend<TRaster::Dimension>(window_impl());
    const auto outK = upto_kth<sizeof...(TFilters) - 2>(in.patch(domain0));
    filter<sizeof...(TFilters) - 1>().transform(outK, out);
  }

  /**
   * @brief Filter an input patch.
   */
  template <typename T, typename TParent, typename TRegion, typename TOut>
  void transform_impl(const Patch<T, TParent, TRegion>& in, TOut& out) const
  {
    const auto& raw = raster(in);
    const auto& domain = in.domain();
    const auto& extrapolate = in.method();
    static constexpr Index N = sizeof...(TFilters);
    const auto domain0 = box(domain) + extend<TParent::Dimension>(window_impl());
    auto outK = upto_kth<N - 2>(extrapolate(raw.patch(domain0)));
    const auto domainK = in.domain() + filter<N - 1>().origin();
    filter<N - 1>().transform(outK.patch(domainK), out);
  }

private:

  template <std::size_t K, typename TIn>
  auto upto_kth(const TIn& in) const
  {
    const auto& domain = in.domain() - extend<TIn::Dimension>(Linx::box(filter<K>().window()));
    const auto patch = in.patch(domain);
    if constexpr (K == 0) {
      return filter<0>() * patch;
    } else {
      return filter<K>() * upto_kth<K - 1>(patch);
    }
  }

private:

  std::tuple<TFilters...> m_filters;
};

/**
 * @brief Combine two filters as a sequence.
 */
template <typename T, typename TWindow, typename TDerived, typename U, typename UWindow, typename UDerived>
auto operator*(FilterMixin<T, TWindow, TDerived> lhs, FilterMixin<U, UWindow, UDerived> rhs)
{
  return FilterSeq<FilterMixin<T, TWindow, TDerived>, FilterMixin<U, UWindow, UDerived>>(
      std::make_tuple(std::move(lhs), std::move(rhs))); // FIXME forward?
}

} // namespace Linx

#endif
