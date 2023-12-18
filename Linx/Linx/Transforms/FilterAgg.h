// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_FILTERAGG_H
#define _LINXTRANSFORMS_FILTERAGG_H

#include "Linx/Base/SeqUtils.h"
#include "Linx/Data/Raster.h"
#include "Linx/Transforms/Extrapolation.h"
#include "Linx/Transforms/impl/Filter.h"

#include <type_traits> // decay

namespace Linx {

/**
 * @brief A sequence of filters.
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

  /// @group_properties

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
    // out.generate(std::forward<TFunc>(m_op), m_filters * in...); // FIXME apply expand m_filters
    out.generate(m_op, std::get<0>(m_filters) * in, std::get<1>(m_filters) * in); // FIXME any number of filters
  }

private:

  TFunc m_op;
  std::tuple<TFilters...> m_filters;
};

} // namespace Linx

#endif
