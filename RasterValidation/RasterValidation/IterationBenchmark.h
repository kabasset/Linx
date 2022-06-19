// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERVALIDATION_ITERATIONBENCHMARK_H
#define _RASTERVALIDATION_ITERATIONBENCHMARK_H

#include "Raster/Raster.h"
#include "RasterValidation/Chronometer.h"

namespace Cnes {

/**
 * @brief Benchmark to measure the different pixel looping strategies.
 */
class IterationBenchmark {

public:
  /**
   * @brief The raster dimension.
   */
  static constexpr Index Dim = 3;

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
  Raster<Value, Dim> m_a;
  Raster<Value, Dim> m_b;
  Raster<Value, Dim> m_c;
  Chronometer<Duration> m_chrono;
};

} // namespace Cnes

#endif // _RASTERVALIDATION_ITERATIONBENCHMARK_H
