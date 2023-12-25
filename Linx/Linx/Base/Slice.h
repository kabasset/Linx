// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXBASE_SLICE_H
#define _LINXBASE_SLICE_H

#include "Linx/Base/TypeUtils.h"

namespace Linx {

/**
 * @brief Linear index spacing.
 * @details
 * A slice has included front and back indices,
 * and an optional step which is the distance between two consecutive indices.
 * The slice size is the number of indices it contains.
 */
class Slice { // FIXME useless?
public:

  /**
   * @brief Constructor.
   * @param front The included front index
   * @param back The included back index
   * @param step The distance between two consecutive indices
   */
  Slice(Index front, Index back, Index step = 1) : m_front(front), m_step(step), m_size((back - front) / step + 1) {}

  /**
   * @brief Make a slice from a front position, number of indices, and optional step.
   */
  static Slice from_size(Index front, std::size_t size, Index step = 1)
  {
    return Slice(front, front + step * (size - 1), step);
  }

  /**
   * @brief Get the included front index.
   */
  Index front() const
  {
    return m_front;
  }

  /**
   * @brief Get the included back index.
   */
  Index back() const
  {
    return m_front + m_step * (m_size - 1);
  }

  /**
   * @brief Get the step.
   */
  Index step() const
  {
    return m_step;
  }

  /**
   * @brief Get the number of indices.
   */
  std::size_t size() const
  {
    return m_size;
  }

  /**
   * @brief Equality operator.
   */
  bool operator==(const Slice& rhs) const
  {
    return m_front == rhs.m_front && m_size == rhs.m_size && m_step == rhs.m_step;
  }

  /**
   * @brief Non-equality operator.
   */
  bool operator!=(const Slice& rhs) const
  {
    return not(*this == rhs);
  }

private:

  Index m_front;
  Index m_step;
  std::size_t m_size;
};

} // namespace Linx

#endif
