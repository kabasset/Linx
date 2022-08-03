// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_SUBRASTER_H
#define _LITLRASTER_SUBRASTER_H

#include "LitlContainer/Arithmetic.h"
#include "LitlContainer/Math.h"
#include "LitlRaster/Box.h"
#include "LitlRaster/Raster.h"
#include "LitlRaster/SubrasterIndexing.h"

#include <algorithm> // accumulate
#include <functional> // multiplies

namespace Litl {

/**
 * @ingroup data_classes
 * @brief A view of a raster region.
 * @tparam T The value type
 * @tparam TParent The parent raster or extrapolator type
 * @tparam TRegion The region type
 * 
 * As opposed to a raster, values of a subraster are generally not contiguous in memory:
 * they are piece-wise contiguous when the region is a `Box`, and sometimes not even piece-wise contiguous.
 * When a region is indeed contiguous, it is better to rely on a `PtrRaster` instead: see `Raster::section()`.
 * 
 * Whatever the region type, subrasters are iterable,
 * and the iterator type depends on the parent and region types in order to maximize performance.
 * Assuming the region itself is cheap to shift, subrasters are cheap to shift and iterate,
 * which makes them ideal to represent sliding windows, even of arbitrary shapes (e.g. when the region is a `Mask`).
 * 
 * In-place pixel-wise operations of rasters (like arithmetic operators and math functions)
 * are applicable to subrasters of mutable parents.
 * 
 * @see pixelwise
 */
template <typename T, typename TParent, typename TRegion = Box<TParent::Dimension>>
class Subraster :
    public ArithmeticMixin<EuclidArithmetic, T, Subraster<T, TParent, TRegion>>,
    public MathFunctionsMixin<T, Subraster<T, TParent, TRegion>>,
    public RangeMixin<T, Subraster<T, TParent, TRegion>> {

public:
  /**
   * @brief The value type.
   */
  using Value = T;
  // std::conditional_t<std::is_const<TParent>::value, const typename TParent::Value, typename TParent::Value>;

  /**
   * @brief The parent type (a raster or extrapolator).
   */
  using Parent = TParent;

  /**
   * @brief The region type.
   */
  using Region = std::decay_t<TRegion>; // FIXME no external reference allowed?

  /**
   * @brief The dimension.
   */
  static constexpr Index Dimension = Parent::Dimension;

  /**
   * @brief The indexing strategy.
   */
  using Indexing = typename SubrasterTraits<std::decay_t<Parent>, Region>::template Indexing<Parent, Region>;

  /**
   * @brief The iterator type.
   */
  template <typename U>
  using Iterator = typename Indexing::template Iterator<U>;

  /// @{
  /// @group_construction

  /**
   * @brief Constructor.
   */
  Subraster(Parent& parent, Region region) :
      m_parent(parent), m_region(std::move(region)), m_indexing(m_parent, m_region) {}
  // std::move(region) not applicable to const Region& // TODO decay?

  /// @group_properties

  /**
   * @brief Get the subraster bounding box.
   */
  Position<Dimension> box() const {
    return box(m_region);
  }

  /**
   * @brief Get the number of pixels in the subraster.
   */
  std::size_t size() const {
    return m_region.size();
  }

  /**
   * @brief Get the region.
   */
  const TRegion& region() const {
    return m_region;
  }

  /**
   * @brief Access the parent raster or extrapolator.
   */
  const Parent& parent() const {
    return m_parent;
  }

  /**
   * @brief Access the parent raster or extrapolator.
   */
  Parent& parent() {
    return m_parent;
  }

  /// @group_elements

  /**
   * @brief Constant iterator to the front pixel.
   */
  Iterator<const Value> begin() const {
    return m_indexing.template begin<const Value>(m_parent, m_region);
  }

  /**
   * @brief Iterator to the front pixel.
   */
  Iterator<Value> begin() {
    return m_indexing.template begin<Value>(m_parent, m_region);
  }

  /**
   * @brief Constant end iterator.
   */
  Iterator<const Value> end() const {
    return m_indexing.template end<const Value>(m_parent, m_region);
  }

  /**
   * @brief End iterator.
   */
  Iterator<Value> end() {
    return m_indexing.template end<Value>(m_parent, m_region);
  }

  /// @group_modifiers

  /**
   * @brief Shift the subraster by a given vector.
   */
  Subraster& shift(const Position<Dimension>& vector) {
    m_region += vector;
    return *this;
  }

  /**
   * @brief Shift the subraster by the opposite of a given vector.
   */
  Subraster& shiftBack(const Position<Dimension>& vector) {
    m_region -= vector;
    return *this;
  }

  /// @}

private:
  /**
   * @brief The parent raster.
   */
  Parent& m_parent;

  /**
   * @brief The region.
   */
  Region m_region;

  /**
   * @brief The shift-invariant indexing helper.
   */
  Indexing m_indexing;
};

} // namespace Litl

#include "LitlRaster/SubrasterIterator.h"

#endif
