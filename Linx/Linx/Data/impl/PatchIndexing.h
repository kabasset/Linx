// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_IMPL_PATCHINDEXING_H
#define _LINXDATA_IMPL_PATCHINDEXING_H

namespace Linx {

/// @cond

template <Index N>
class Box;

template <Index N>
class Grid;

template <Index I, Index N>
class Line;

template <Index N>
class Mask;

// Forward declaration for specializations
template <typename T, Index N, typename THolder>
class Raster;

/// @endcond

/**
 * @brief Indexing based on (contiguous) pointers.
 */
template <typename TParent, typename TRegion>
class ContiguousIndexing {
public:

  template <typename T>
  using Iterator = T*;

  ContiguousIndexing() {}

  ContiguousIndexing(const TParent&, const TRegion&) {}

  template <typename T>
  Iterator<T> begin(TParent& parent, const TRegion& region) const
  {
    return &parent[*region.begin()];
  }

  template <typename T>
  Iterator<T> end(TParent& parent, const TRegion& region) const
  {
    return begin + region.size();
  }
};

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
   * @brief Default constructor.
   */
  PositionBasedIndexing() {}

  /**
   * @brief Constructor.
   */
  PositionBasedIndexing(const TParent&, const TRegion&) {}

  /**
   * @brief Get an iterator to the beginning.
   */
  template <typename T>
  Iterator<T> begin(TParent& parent, const TRegion& region) const
  {
    return Iterator<T>(parent, region.begin());
  }

  /**
   * @brief Get an iterator to the end.
   */
  template <typename T>
  Iterator<T> end(TParent& parent, const TRegion& region) const
  {
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
   * @brief Default constructor.
   */
  StrideBasedIndexing() : m_step(1), m_width(0), m_offsets(1) {}

  /**
   * @brief Constructor for boxes.
   */
  StrideBasedIndexing(const TParent& parent, const Box<TParent::Dimension>& region) :
      m_step(1), m_width(region.template length<0>()), m_offsets(region.size() / std::max(m_width, 1L) + 1)
  // max to prevent division by 0, +1 in order to dereference m_offsets.end() in iterator
  {
    if (region.size() <= 0) { // FIXME needed?
      return;
    }
    auto plane = (region - region.front()).project();
    std::transform(plane.begin(), plane.end(), m_offsets.begin(), [&](const auto& p) {
      return parent.index(p);
    });
  }

  /**
   * @brief Constructor for grids.
   */
  StrideBasedIndexing(const TParent& parent, const Grid<TParent::Dimension>& region) :
      m_step(region.step()[0]), m_width(region.template length<0>()),
      m_offsets(region.size() / std::max(m_width, 1L) + 1)
  // for max and +1 see above
  {
    if (region.size() <= 0) { // FIXME needed?
      return;
    }
    const auto plane = (region - region.front()).project();
    std::transform(plane.begin(), plane.end(), m_offsets.begin(), [&](const auto& p) {
      return parent.index(p);
    });
  }

  /**
   * @brief Constructor for lines.
   */
  template <Index I>
  StrideBasedIndexing(const TParent& parent, const Line<I, TParent::Dimension>& region) :
      m_step(shape_stride<I>(parent.shape()) * region.step()), m_width(m_step * (region.size() - 1) + 1),
      m_offsets(2, 0)
  {} // 1+1 see above

  /**
   * @brief Get an iterator to the beginning.
   */
  template <typename T>
  Iterator<T> begin(TParent& raster, const TRegion& region) const
  {
    return Iterator<T>(&raster[region.front()], m_step, m_width, m_offsets.data());
  }

  /**
   * @brief Get an iterator to the end.
   */
  template <typename T>
  Iterator<T> end(TParent& raster, const TRegion& region) const
  {
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
   * @brief Default constructor.
   */
  OffsetBasedIndexing() : m_offsets(1) {}

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
  Iterator<T> begin(TParent& raster, const TRegion& region) const
  {
    return Iterator<T>(&raster[box(region).front()], m_offsets.data());
  }

  /**
   * @brief Get an iterator to the end.
   */
  template <typename T>
  Iterator<T> end(TParent& raster, const TRegion& region) const
  {
    return Iterator<T>(&raster[box(region).front()], m_offsets.data() + m_offsets.size() - 1);
  }

private:

  /**
   * @brief The offsets relative to the front index.
   */
  std::vector<Index> m_offsets;
};

template <typename TParent, typename TRegion, bool IsContiguous = false>
struct PatchTraits {
  /**
   * @brief The indexing strategy of a patch, depending on the type of parent and region.
   * 
   * As opposed to the patch itself, the indexing is translation-invariant.
   * Optimized regions for raster parents are:
   * - `Box`,
   * - `Grid`,
   * - `Line`,
   * - `Mask`.
   * 
   * For extrapolators, no optimization is performed.
   */
  template <typename UParent, typename URegion>
  using Indexing = PositionBasedIndexing<UParent, URegion>;
};

/**
 * @brief Contiguous view specialization.
 */
template <typename TParent, typename TRegion>
struct PatchTraits<TParent, TRegion, true> {
  template <typename UParent, typename URegion>
  using Indexing = ContiguousIndexing<UParent, URegion>;
};

/// @cond

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
 * @brief `Line` specialization.
 */
template <typename T, Index N, typename THolder, Index I>
struct PatchTraits<Raster<T, N, THolder>, Line<I, N>> {
  template <typename UParent, typename URegion>
  using Indexing = StrideBasedIndexing<UParent, URegion>;
};

/**
 * @brief `Mask` specialization.
 */
template <typename T, Index N, typename THolder>
struct PatchTraits<Raster<T, N, THolder>, Mask<N>> {
  template <typename UParent, typename URegion>
  using Indexing = OffsetBasedIndexing<UParent, URegion>;
};

/// @endcond

} // namespace Linx

#include "Linx/Data/impl/PatchIterator.h"

#endif
