// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_FILTERAGG_H
#define _LINXTRANSFORMS_FILTERAGG_H

#include "Linx/Base/SeqUtils.h"
#include "Linx/Data/Raster.h"
#include "Linx/Transforms/Extrapolation.h"
#include "Linx/Transforms/mixins/Filter.h"

#include <type_traits> // decay

namespace Linx {

/**
 * @ingroup filtering
 * @brief An aggregate of filters.
 */
template <typename TFunc, typename... TFilters>
class FilterAgg :
    public FilterMixin<
        typename std::invoke_result_t<TFunc, typename TFilters::Value...>,
        Box<std::max({TFilters::Dimension...})>,
        FilterAgg<TFunc, TFilters...>> {
  friend class FilterMixin<
      typename std::invoke_result_t<TFunc, typename TFilters::Value...>,
      Box<std::max({TFilters::Dimension...})>,
      FilterAgg<TFunc, TFilters...>>; // FIXME simplify

public:

  /**
   * @brief The aggregation value type.
   */
  using Value = typename std::invoke_result_t<TFunc, typename TFilters::Value...>;

  /**
   * @brief The logical dimension of the composed kernel.
   */
  static constexpr Index Dimension = std::max({TFilters::Dimension...});

  /**
   * @brief Constructor.
   * 
   * @note Filter aggregates are better constructed by calling operators on filters.
   */
  explicit FilterAgg(TFunc&& op, TFilters&&... filters) :
      m_op(std::forward<TFunc>(op)), m_filters(std::forward<TFilters>(filters)...)
  {}

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
      const auto& w = k.window();
      for (Index i = 0; i < w.dimension(); ++i) {
        front[i] = std::min(front[i], w.front()[i]);
        back[i] = std::max(back[i], w.back()[i]);
      }
    });
    return {front, back};
  }

  /**
   * @brief Apply the filters to an input extrapolator.
   */
  template <typename TIn, typename TOut>
  void transform_impl(const TIn& in, TOut& out) const
  {
    transform_impl(in, out, std::make_index_sequence<sizeof...(TFilters)> {});
  }

private:

  template <typename TIn, typename TOut, std::size_t... Is>
  void transform_impl(const TIn& in, TOut& out, std::index_sequence<Is...>) const
  {
    out.generate(m_op, std::get<Is>(m_filters) * in...);
  }

private:

  TFunc m_op;
  std::tuple<TFilters...> m_filters;
};

/**
 * @ingroup filtering
 * @relatesalso FilterAgg
 * @brief Sum two filters.
 */
template <
    typename TFilter,
    typename UFilter,
    typename std::enable_if_t<is_filter<TFilter>() && is_filter<UFilter>()>* = nullptr>
auto operator+(const TFilter& lhs, const UFilter& rhs)
{
  return FilterAgg<TFilter, UFilter>(std::plus<typename TFilter::Value>(), lhs, rhs);
}

/**
 * @ingroup filtering
 * @relatesalso FilterAgg
 * @brief Get the P-norm of several filters.
 */
template <
    Index P = 2,
    typename TFilter0,
    typename std::enable_if_t<is_filter<TFilter0>()>* = nullptr,
    typename... TFilters>
auto norm(const TFilter0& filter0, const TFilters&... filters)
{
  auto agg = [&](const typename TFilter0::Value& e0, const typename TFilters::Value&... es) {
    return abspow<P>(e0) + (abspow<P>(es) + ...);
  };
  return FilterAgg<decltype(agg), TFilter0, TFilters...>(agg, filter0, filters...);
}

} // namespace Linx

#endif
