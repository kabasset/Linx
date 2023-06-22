// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_PATCH_H
#define _LINXDATA_PATCH_H

#include "Linx/Base/Arithmetic.h"
#include "Linx/Base/Math.h"
#include "Linx/Data/Box.h"
#include "Linx/Data/Raster.h"
#include "Linx/Data/impl/PatchIndexing.h"

#include <algorithm> // accumulate
#include <functional> // multiplies

namespace Linx {

/**
 * @ingroup data_classes
 * @ingroup regions
 * @brief A view of a raster region.
 * @tparam T The value type
 * @tparam TParent The parent raster or extrapolator type
 * @tparam TRegion The region type
 * 
 * As opposed to a raster, values of a patch are generally not contiguous in memory:
 * they are piece-wise contiguous when the region is a `Box`, and sometimes not even piece-wise contiguous.
 * When a region is indeed contiguous, it is better to rely on a `PtrRaster` instead: see `Raster::section()`.
 * 
 * Whatever the region type, patches are iterable,
 * and the iterator type depends on the parent and region types in order to maximize performance.
 * Assuming the region itself is cheap to translate, patches are cheap to translate and iterate,
 * which makes them ideal to represent sliding windows, even of arbitrary shapes (e.g. when the region is a `Mask`).
 * 
 * In-place pixel-wise operations of rasters (like arithmetic operators and math functions)
 * are applicable to patches of mutable parents.
 * 
 * @see pixelwise
 */
template <typename T, typename TParent, typename TRegion = Box<TParent::Dimension>>
class Patch :
    public ArithmeticMixin<EuclidArithmetic, T, Patch<T, TParent, TRegion>>,
    public MathFunctionsMixin<T, Patch<T, TParent, TRegion>>,
    public RangeMixin<T, Patch<T, TParent, TRegion>> {

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
  using Indexing = typename PatchTraits<std::decay_t<Parent>, Region>::template Indexing<Parent, Region>;

  /**
   * @brief The iterator type.
   */
  template <typename U>
  using Iterator = typename Indexing::template Iterator<U>;

  /// @{
  /// @group_construction

  /**
   * @brief Default constructor.
   * 
   * This is a dummy constructor defined for compatibility with `std::vector`.
   */
  Patch() : m_parent(nullptr), m_region(), m_indexing() {}

  /**
   * @brief Constructor.
   */
  Patch(Parent& parent, Region region) :
      m_parent(&parent), m_region(std::move(region)), m_indexing(*m_parent, m_region) {}
  // std::move(region) not applicable to const Region& // TODO decay?

  /// @group_properties

  /**
   * @brief Get the patch bounding box.
   */
  Box<Dimension> box() const {
    return box(m_region);
  }

  /**
   * @brief Get the number of pixels in the patch.
   */
  std::size_t size() const {
    return m_region.size();
  }

  /**
   * @brief Get the region.
   */
  const TRegion& domain() const {
    return m_region;
  }

  /**
   * @brief Access the parent raster or extrapolator.
   */
  const Parent& parent() const {
    return *m_parent;
  }

  /**
   * @brief Access the parent raster or extrapolator.
   */
  Parent& parent() {
    return *m_parent;
  }

  /// @group_elements

  /**
   * @brief Access the element at given position in the region referential.
   * 
   * Only patches based on regions with suitable `operator[]()` are supported.
   * For example, box-based patches support `Position` as an argument,
   * while line-based patches support `Index` as an argument.
   * 
   * This method is provided for convenience but is slower than iterators.
   * Generally, it should be avoided in hot paths.
   */
  template <typename TPos>
  const Value& operator[](const TPos& pos) const {
    return (*m_parent)[m_region[pos]];
  }

  /**
   * @copybrief operator[]()const
   */
  template <typename TPos>
  Value& operator[](const TPos& pos) {
    return (*m_parent)[m_region[pos]];
  }

  /**
   * @brief Constant iterator to the front pixel.
   */
  Iterator<const Value> begin() const {
    return m_indexing.template begin<const Value>(*m_parent, m_region);
  }

  /**
   * @brief Iterator to the front pixel.
   */
  Iterator<Value> begin() {
    return m_indexing.template begin<Value>(*m_parent, m_region);
  }

  /**
   * @brief Constant end iterator.
   */
  Iterator<const Value> end() const {
    return m_indexing.template end<const Value>(*m_parent, m_region);
  }

  /**
   * @brief End iterator.
   */
  Iterator<Value> end() {
    return m_indexing.template end<Value>(*m_parent, m_region);
  }

  /// @group_modifiers

  /**
   * @brief Translate the patch by a given vector.
   */
  Patch& translate(const Position<Dimension>& vector) {
    m_region += vector;
    return *this;
  }

  /**
   * @brief Translate the patch by the opposite of a given vector.
   */
  Patch& translateBack(const Position<Dimension>& vector) {
    m_region -= vector;
    return *this;
  }

  /// @group_operations

  /**
   * @brief Check whether two masks are equal.
   */
  bool operator==(const Patch<T, TParent, TRegion>& other) const {
    return m_parent == other.m_parent && m_region == other.m_region;
  }

  /**
   * @brief Create a cropped patch.
   */
  Patch<const T, const TParent, TRegion> patch(const Box<TParent::Dimension>& box) const {
    return Patch<const T, const TParent, TRegion>(*m_parent, m_region & box);
  }

  /**
   * @brief Create a cropped patch.
   */
  Patch<T, TParent, TRegion> patch(const Box<TParent::Dimension>& box) {
    return Patch<T, TParent, TRegion>(*m_parent, m_region & box);
  }

  /// @}

private:
  /**
   * @brief The parent raster.
   */
  Parent* m_parent;

  /**
   * @brief The region.
   */
  Region m_region;

  /**
   * @brief The translation-invariant indexing helper.
   */
  Indexing m_indexing;
};

/// @cond
namespace Internal {
template <typename T>
struct IsPatchImpl : std::false_type {};

template <typename T, typename TParent, typename TRegion>
struct IsPatchImpl<Patch<T, TParent, TRegion>> : std::true_type {};

} // namespace Internal
/// @endcond

template <typename T>
constexpr bool isPatch() {
  return Internal::IsPatchImpl<T>::value;
}

/**
 * @relatesalso Patch
 * @brief Get the parent raster of a patch.
 * 
 * As opposed to `Patch::parent()`, if the parent is an extrapolator,
 * then the underlying decorated raster is effectively returned.
 */
template <typename T, typename TParent, typename TRegion>
const auto& rasterize(const Patch<T, TParent, TRegion>& in) {
  return rasterize(in.parent());
}

} // namespace Linx

#endif
