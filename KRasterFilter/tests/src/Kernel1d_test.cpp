// Copyright (C) 2022, Antoine Basset
// This file is part of Kast.Raster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "KRasterFilter/Kernel1d.h"

#include <boost/test/unit_test.hpp>

using namespace Kast;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Kernel1d_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(sum_kernel_test) {
  const Kernel1d<int> one({1, 1, 1}, 1);
  const auto separable = one.along<0, 1, 2>();
  VecRaster<int, 3> raster({3, 3, 3});
  std::fill(raster.begin(), raster.end(), 1);
  const auto sum = separable.correlate<int>(raster);
  const std::vector<int> expected {
      8,  12, 8,  12, 18, 12, 8,  12, 8, // z = 0
      12, 18, 12, 18, 27, 18, 12, 18, 12, // z = 1
      8,  12, 8,  12, 18, 12, 8,  12, 8}; // z = 2
  for (std::size_t i = 0; i < 3 * 3 * 3; ++i) {
    BOOST_TEST(sum[i] == expected[i]);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
