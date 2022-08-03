// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Subraster_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(mixins_test) {

  // Setup
  Raster<int, 1> raster({16});
  raster.range();
  Box<1> region {{1}, {14}};
  auto subraster = raster.subraster(region);

  // Arithmetic
  ++subraster;

  // MathFunctions
  subraster.pow(2);

  // Range
  subraster.apply([](auto v) {
    return -v;
  });

  // Test
  BOOST_TEST(raster[0] == 0);
  BOOST_TEST(raster[15] == 15);
  for (Index i = 1; i <= 14; ++i) {
    BOOST_TEST(raster[i] == -(i + 1) * (i + 1));
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
