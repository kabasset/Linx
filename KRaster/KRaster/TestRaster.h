// Copyright (C) 2022, CNES
// This file is part of KRaster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _KRASTER_TESTRASTER_H
#define _KRASTER_TESTRASTER_H

#include "KRaster/Raster.h"
#include "KRaster/TestUtils.h"

namespace Cnes {
namespace Test {

/**
 * @brief A random Raster of given type and shape.
 * @details
 * Values are uniformly distributed between a given min and max.
 */
template <typename T, long N = 2>
class RandomRaster : public VecRaster<T, N> {

public:
  /**
   * @brief Generate a Raster with given shape.
   */
  explicit RandomRaster(Position<N> shape, T min = almostMin<T>(), T max = almostMax<T>()) :
      VecRaster<T, N>(shape, generateRandomVector<T>(shapeSize(shape), min, max)) {}

  /**
   * @brief Destructor.
   */
  virtual ~RandomRaster() = default;

  /**
   * @brief Check whether the raster is approximately equal to a reference raster.
   * @details
   * Test each pixel as: |ref - test| / test < tol
   */
  template <typename TRaster>
  bool approx(const TRaster& ref, double tol = 0.01) const {
    if (this->shape() != ref.shape()) {
      return false;
    }
    for (std::size_t i = 0; i < this->size(); ++i) {
      if (not approx(*this[i], ref[i], tol)) {
        return false;
      }
    }
    return true;
  }
};

/**
 * @brief Check whether a test raster is approximately equal to a reference raster.
 * @details
 * Test each pixel as: |ref - test| / test < tol
 */
template <typename TRaster, typename URaster>
bool rasterApprox(const TRaster& test, const URaster& ref, double tol = 0.01);

} // namespace Test
} // namespace Cnes

#endif // _KRASTER_TESTRASTER_H
