// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RasterFilter/Kernel1d.h"

#include <boost/test/unit_test.hpp>

using namespace Cnes;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Kernel1d_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(combination_test) {
  const auto combined = sobel<int, 0, 1>().combine();
  std::vector<int> values {1, 0, -1, 2, 0, -2, 1, 0, -1};
  Raster<int, 2> expected({3, 3}, std::move(values));
  BOOST_TEST(combined.shape() == expected.shape());
  BOOST_TEST(combined.container() == expected.container());
}

BOOST_AUTO_TEST_CASE(sum_kernel_test) {
  const Kernel1d<int> one({1, 1, 1}, 1);
  const auto separable = one.along<0, 1, 2>();
  Raster<int, 3> raster({3, 3, 3});
  raster.fill(1);
  const auto sum = separable.correlate<int>(raster);
  const std::vector<int> expected {
      8,  12, 8,  12, 18, 12, 8,  12, 8, // z = 0
      12, 18, 12, 18, 27, 18, 12, 18, 12, // z = 1
      8,  12, 8,  12, 18, 12, 8,  12, 8}; // z = 2
  for (std::size_t i = 0; i < 3 * 3 * 3; ++i) {
    BOOST_TEST(sum[i] == expected[i]);
  }
}

BOOST_AUTO_TEST_CASE(make_sobel_test) {
  const auto sobelX = sobel<int, 0, 1>();
  const auto sobelY = sobel<int, 1, 0>();
  Raster<int, 3> raster({3, 3, 3});
  raster.fill(1);
  const auto edgesX = sobelX.correlate<int>(raster);
  const auto edgesY = sobelY.correlate<int>(raster);
  const std::vector<int> expectedX {
      -3, 0, 3, -4, 0, 4, -3, 0, 3, // z = 0
      -3, 0, 3, -4, 0, 4, -3, 0, 3, // z = 1
      -3, 0, 3, -4, 0, 4, -3, 0, 3}; // z = 2
  const std::vector<int> expectedY {
      -3, -4, -3, 0, 0, 0, 3, 4, 3, // z = 0
      -3, -4, -3, 0, 0, 0, 3, 4, 3, // z = 1
      -3, -4, -3, 0, 0, 0, 3, 4, 3}; // z = 2
  Index i = 0;
  for (Index y = 0; y < 3; ++y) {
    for (Index x = 0; x < 3; ++x, ++i) {
      printf("%i/%i ", edgesX[{x, y, 0}], expectedX[i]);
    }
    printf("\n");
  }
  for (std::size_t i = 0; i < 3 * 3 * 3; ++i) {
    BOOST_TEST(edgesX[i] == expectedX[i]);
    BOOST_TEST(edgesY[i] == expectedY[i]);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
