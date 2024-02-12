// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXRUN_ITERATIONBENCHMARK_H
#define _LINXRUN_ITERATIONBENCHMARK_H

#include "Linx/Data/Raster.h"
#include "Linx/Run/Timer.h"

namespace Linx {

/**
 * @brief Benchmark to measure the different pixel looping strategies.
 */
class IterationBenchmark {
public:

  /**
   * @brief The raster dimension.
   */
  static constexpr Index Dimension = 3;

  /**
   * @brief The raster value type.
   */
  using Value = char;

  /**
   * @brief The duration unit.
   */
  using Duration = std::chrono::milliseconds;

  /**
   * @brief Constructor.
   */
  IterationBenchmark(Index side);

  /**
   * @brief Loop over positions built by looping over x, then y, and then z.
   */
  Duration loop_over_xyz();

  /**
   * @brief Loop over positions built by looping over z, then y, and then x.
   */
  Duration loop_over_zyx();

  /**
   * @brief Loop over positions via a position iterator.
   */
  Duration iterate_over_positions();

  /**
   * @brief Loop over positions via a position iterator and factorized index computation.
   */
  Duration iterate_over_positions_optimized();

  /**
   * @brief Loop over indices.
   */
  Duration loop_over_indices();

  /**
   * @brief Loop over values via a pixel iterator.
   */
  Duration iterate_over_values();

  /**
   * @brief Call buitin operator.
   */
  Duration call_operator();

  /**
   * @brief Call `Raster::generate()`.
   */
  Duration call_generate();

protected:

  Index m_width;
  Index m_height;
  Index m_depth;
  Raster<Value, Dimension> m_a;
  Raster<Value, Dimension> m_b;
  Raster<Value, Dimension> m_c;
  Timer<Duration> m_timer;
};

} // namespace Linx

#endif
