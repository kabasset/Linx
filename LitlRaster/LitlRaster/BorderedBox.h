/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_BORDEREDBOX_H
#define _LITLRASTER_BORDEREDBOX_H

#include "LitlRaster/Box.h"

#include <deque>
#include <vector>

namespace Litl {

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
  BorderedBox(Box<N> box, Box<N> margin) : m_inner(std::move(box)), m_fronts(), m_backs() {

    const auto dim = m_inner.dimension();
    m_fronts.reserve(dim);
    for (Index i = 0; i < dim; ++i) {

      const auto f = margin.front()[i];
      if (f < 0) {
        auto before = m_inner;
        before.m_back[i] = m_inner.m_front[i] - 1;
        before.m_front[i] = m_inner.m_front[i] += f;
        m_fronts.push_back(before);
      }

      const auto b = margin.back()[i];
      if (b > 0) {
        auto after = m_inner;
        after.m_front[i] = m_inner.m_back[i] + 1;
        after.m_back[i] = m_inner.m_back[i] += b;
        m_backs.push_front(after);
      }
    }
  }

  /// @group_operations

  /**
   * @brief Apply two different functions to the inner and bordering boxes.
   */
  template <typename TInnerFunc, typename TBorderFunc>
  void applyInnerBorder(TInnerFunc&& innerFunc, TBorderFunc&& borderFunc) const {
    for (const auto& box : m_fronts) {
      std::forward<TBorderFunc>(borderFunc)(box);
    }
    std::forward<TInnerFunc>(innerFunc)(m_inner);
    for (const auto& box : m_backs) {
      std::forward<TBorderFunc>(borderFunc)(box);
    }
  }

  /// @}

private:
  Box<N> m_inner;
  std::vector<Box<N>> m_fronts;
  std::deque<Box<N>> m_backs;
};

} // namespace Litl

#endif
