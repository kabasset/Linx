/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_BORDEREDBOX_H
#define _LINXDATA_BORDEREDBOX_H

#include "Linx/Data/Box.h"

#include <deque>
#include <vector>

namespace Linx {

/// @cond
namespace Internal {

/**
 * @ingroup regions
 * @brief A box made of an inner box and bordering boxes.
 */
template <Index N = 2>
class BorderedBox {

public:
  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = N;

  /// @{
  /// @group_construction

  /**
   * @brief Constructor.
   */
  BorderedBox(const Box<N>& box, const Box<N>& margin) : m_inner(box - margin), m_fronts(), m_backs() {

    auto current = m_inner;
    const auto dim = m_inner.dimension();
    m_backs.reserve(dim);
    for (Index i = 0; i < dim; ++i) {

      const auto f = margin.front()[i];
      if (f < 0) {
        auto before = current;
        before.m_back[i] = current.m_front[i] - 1;
        before.m_front[i] = current.m_front[i] += f;
        m_fronts.push_front(before);
      }

      const auto b = margin.back()[i];
      if (b > 0) {
        auto after = current;
        after.m_front[i] = current.m_back[i] + 1;
        after.m_back[i] = current.m_back[i] += b;
        m_backs.push_back(after);
      }
    }
  }

  /// @group_operations

  /**
   * @brief Apply two different functions to the inner and bordering boxes.
   */
  template <typename TInnerFunc, typename TBorderFunc>
  void apply_inner_border(TInnerFunc&& inner_func, TBorderFunc&& border_func) const {
    for (const auto& box : m_fronts) {
      std::forward<TBorderFunc>(border_func)(box);
    }
    std::forward<TInnerFunc>(inner_func)(m_inner);
    for (const auto& box : m_backs) {
      std::forward<TBorderFunc>(border_func)(box);
    }
  }

  /// @}

private:
  Box<N> m_inner;
  std::deque<Box<N>> m_fronts;
  std::vector<Box<N>> m_backs;
};

} // namespace Internal
/// @endcond

} // namespace Linx

#endif
