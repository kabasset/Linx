// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXRUN_ITERATIONBENCHMARK_H
#define _LINXRUN_ITERATIONBENCHMARK_H

#include "LinxCore/Raster.h"
#include "LinxRun/Chronometer.h"

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
  Duration loopOverXyz();

  /**
   * @brief Loop over positions built by looping over z, then y, and then x.
   */
  Duration loopOverZyx();

  /**
   * @brief Loop over positions via a position iterator.
   */
  Duration iterateOverPositions();

  /**
   * @brief Loop over positions via a position iterator and factorized index computation.
   */
  Duration iterateOverPositionsOptimized();

  /**
   * @brief Loop over indices.
   */
  Duration loopOverIndices();

  /**
   * @brief Loop over values via a pixel iterator.
   */
  Duration iterateOverValues();

  /**
   * @brief Call buitin operator.
   */
  Duration callOperator();

  /**
   * @brief Call `Raster::generate()`.
   */
  Duration callGenerate();

protected:
  Index m_width;
  Index m_height;
  Index m_depth;
  Raster<Value, Dimension> m_a;
  Raster<Value, Dimension> m_b;
  Raster<Value, Dimension> m_c;
  Chronometer<Duration> m_chrono;
};

} // namespace Linx

#endif
