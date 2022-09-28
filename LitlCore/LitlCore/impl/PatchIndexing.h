// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLCORE_IMPL_PATCHINDEXING_H
#define _LITLCORE_IMPL_PATCHINDEXING_H

#include "LitlBase/Sequence.h"
#include "LitlCore/Box.h"
#include "LitlCore/Grid.h"
#include "LitlCore/Mask.h"
#include "LitlCore/OrientedSlice.h"
#include "LitlCore/Raster.h"

namespace Litl {

/**
 * @brief Indexing based only on positions.
 */
template <typename TParent, typename TRegion>
class PositionBasedIndexing {
public:
  /**
   * @brief The patch iterator.
   */
  template <typename T>
  class Iterator;

  /**
   * @brief Constructor.
   */
  PositionBasedIndexing(const TParent&, const TRegion&) {}

  /**
   * @brief Get an iterator to the beginning.
   */
  template <typename T>
  Iterator<T> begin(TParent& parent, const TRegion& region) const {
    return Iterator<T>(parent, region.begin());
  }

  /**
   * @brief Get an iterator to the end.
   */
  template <typename T>
  Iterator<T> end(TParent& parent, const TRegion& region) const {
    return Iterator<T>(parent, region.end());
  }
};

/**
 * @brief Indexing of regular grids, based on strides.
 */
template <typename TParent, typename TRegion>
class StrideBasedIndexing {
public:
  /**
   * @brief The patch iterator.
   */
  template <typename T>
  class Iterator;

  /**
   * @brief Constructor for boxes.
   */
  StrideBasedIndexing(const TParent& parent, const Box<TParent::Dimension>& region) :
      m_step(1), m_width(region.template length<0>()),
      m_offsets(region.size() / m_width + 1) // +1 in order to dereference m_offsets.end() in iterator
  {
    auto plane = (region - region.front()).project();
    std::transform(plane.begin(), plane.end(), m_offsets.begin(), [&](const auto& p) {
      return parent.index(p);
    });
  }

  /**
   * @brief Constructor for grids.
   */
  StrideBasedIndexing(const TParent& parent, const Grid<TParent::Dimension>& region) :
      m_step(region.step()[0]), m_width(box(region).template length<0>()),
      m_offsets(region.size() / m_width + 1) // +1 see above
  {
    auto plane = (region - region.front()).project();
    std::transform(plane.begin(), plane.end(), m_offsets.begin(), [&](const auto& p) {
      return parent.index(p);
    });
  }

  /**
   * @brief Constructor for slices.
   */
  template <Index I>
  StrideBasedIndexing(const TParent& parent, const OrientedSlice<I, TParent::Dimension>& region) :
      m_step(shapeStride<I>(parent.shape()) * region.step()), m_width(m_step * (region.size() - 1) + 1),
      m_offsets(2, 0) {} // 1+1 see above

  /**
   * @brief Get an iterator to the beginning.
   */
  template <typename T>
  Iterator<T> begin(TParent& raster, const TRegion& region) const {
    return Iterator<T>(&raster[region.front()], m_step, m_width, m_offsets.data());
  }

  /**
   * @brief Get an iterator to the end.
   */
  template <typename T>
  Iterator<T> end(TParent& raster, const TRegion& region) const {
    return Iterator<T>(&raster[region.front()], m_step, m_width, m_offsets.data() + m_offsets.size() - 1);
  }

private:
  /**
   * @brief The stride along axis 0.
   */
  Index m_step;

  /**
   * @brief The length along axis 0.
   */
  Index m_width;

  /**
   * @brief The offsets relative to the region front.
   */
  std::vector<Index> m_offsets;
};

/**
 * @brief Indexing of irregular regions, based on offsets.
 */
template <typename TParent, typename TRegion>
class OffsetBasedIndexing {
public:
  /**
   * @brief The patch iterator.
   */
  template <typename T>
  class Iterator;

  /**
   * @brief Constructor.
   */
  OffsetBasedIndexing(const TParent& parent, const TRegion& region) :
      m_offsets(region.size() + 1) // +1 in order to dereference m_offsets.end() in iterator
  {
    const auto front = box(region).front();
    std::transform(region.begin(), region.end(), m_offsets.begin(), [&](const auto& p) {
      return parent.index(p - front);
    });
  }

  /**
   * @brief Get an iterator to the beginning.
   */
  template <typename T>
  Iterator<T> begin(TParent& raster, const TRegion& region) const {
    return Iterator<T>(&raster[box(region).front()], m_offsets.data());
  }

  /**
   * @brief Get an iterator to the end.
   */
  template <typename T>
  Iterator<T> end(TParent& raster, const TRegion& region) const {
    return Iterator<T>(&raster[box(region).front()], m_offsets.data() + m_offsets.size() - 1);
  }

private:
  /**
   * @brief The offsets relative to the front index.
   */
  std::vector<Index> m_offsets;
};

template <typename TParent, typename TRegion>
struct PatchTraits {
  /**
   * @brief The indexing strategy of a patch, depending on the type of parent and region.
   * 
   * As opposed to the patch itself, the indexing is shift-invariant.
   * Optimized regions for raster parents are:
   * - `Box`,
   * - `Grid`,
   * - `OrientedSlice`
   * - `Mask`.
   * 
   * For extrapolators, no optimization is performed.
   */
  template <typename UParent, typename URegion>
  using Indexing = PositionBasedIndexing<UParent, URegion>;
};

/**
 * @brief `Box` specialization.
 */
template <typename T, Index N, typename THolder>
struct PatchTraits<Raster<T, N, THolder>, Box<N>> {
  template <typename UParent, typename URegion>
  using Indexing = StrideBasedIndexing<UParent, URegion>;
};

/**
 * @brief `Grid` specialization.
 */
template <typename T, Index N, typename THolder>
struct PatchTraits<Raster<T, N, THolder>, Grid<N>> {
  template <typename UParent, typename URegion>
  using Indexing = StrideBasedIndexing<UParent, URegion>;
};

/**
 * @brief `OrientedSlice` specialization.
 */
template <typename T, Index N, typename THolder, Index I>
struct PatchTraits<Raster<T, N, THolder>, OrientedSlice<I, N>> {
  template <typename UParent, typename URegion>
  using Indexing = StrideBasedIndexing<UParent, URegion>;
};

/// @cond
// FIXME temporary workaround for
// 'Mask' was not declared in this scope
// (include issue?)
template <Index N>
class Mask;
/// @endcond

/**
 * @brief `Mask` specialization.
 */
template <typename T, Index N, typename THolder>
struct PatchTraits<Raster<T, N, THolder>, Mask<N>> {
  template <typename UParent, typename URegion>
  using Indexing = OffsetBasedIndexing<UParent, URegion>;
};

} // namespace Litl

#include "LitlCore/impl/PatchIterator.h"

#endif
