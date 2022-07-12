// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlTransforms/SeparableKernel.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(SeparableKernel_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(composition_test) {
  const auto combined = SeparableKernel<int, 0, 1>::sobel().compose();
  std::vector<int> values {1, 0, -1, 2, 0, -2, 1, 0, -1};
  Raster<int, 2> expected({3, 3}, std::move(values));
  std::cout << combined.raster() << std::endl;
  std::cout << expected << std::endl;
  BOOST_TEST(combined.raster() == expected);
}

BOOST_AUTO_TEST_CASE(associativity_commutativity_test) {
  const auto a = LineKernel<int>({1, 0, -1}).along<0>();
  const auto b = LineKernel<int>({1, 2, 1}).along<1>();
  const auto c = a * b;
  const auto raster = Raster<int, 3>({3, 3, 3}).range();
  const auto direct = c * raster;
  const auto associated = a * b * raster;
  const auto commutated = b * a * raster;
  BOOST_TEST(associated == direct);
  BOOST_TEST(commutated == direct);
}

BOOST_AUTO_TEST_CASE(sum_kernel_test) {
  const LineKernel<int> one({1, 1, 1}, 1);
  const auto separable = one.along<0, 1, 2>();
  const auto raster = Raster<int, 3>({3, 3, 3}).fill(1);
  const auto sum = separable * raster;
  const std::vector<int> expected {
      8,  12, 8,  12, 18, 12, 8,  12, 8, // z = 0
      12, 18, 12, 18, 27, 18, 12, 18, 12, // z = 1
      8,  12, 8,  12, 18, 12, 8,  12, 8}; // z = 2
  for (std::size_t i = 0; i < 3 * 3 * 3; ++i) {
    BOOST_TEST(sum[i] == expected[i]);
  }
}

BOOST_AUTO_TEST_CASE(make_sobel_test) {
  const auto sobelX = SeparableKernel<int, 0, 1>::sobel();
  const auto sobelY = SeparableKernel<int, 1, 0>::sobel();
  const auto raster = Raster<int, 3>({3, 3, 3}).fill(1);
  const auto edgesX = sobelX * raster;
  const auto edgesY = sobelY * raster;
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
  for (i = 0; i < 3 * 3 * 3; ++i) {
    BOOST_TEST(edgesX[i] == expectedX[i]);
    BOOST_TEST(edgesY[i] == expectedY[i]);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
