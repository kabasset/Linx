// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LinxTransforms/Interpolation.h"
#include "LinxTransforms/OrientedKernel.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(OrientedKernel_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(sum3_neumann_test) {
  const auto raster = Raster<int>({3, 3}).range();
  const auto in = extrapolate(raster, 0);
  const auto kernel0 = OrientedKernel<int, 0>({1, 1, 1});
  const auto kernel1 = OrientedKernel<int, 1>({1, 1, 1});
  const auto out0 = kernel0 * in;
  const auto out1 = kernel1 * in;
  // 0 1 2 //  1  3  3 // 3  5  7
  // 3 4 5 //  7 12  9 // 9 12 15
  // 6 7 8 // 13 21 15 // 9 11 13
  const std::vector<int> expected0 {1, 3, 3, 7, 12, 9, 13, 21, 15};
  const std::vector<int> expected1 {3, 5, 7, 9, 12, 15, 9, 11, 13};
  BOOST_TEST(out0.shape() == raster.shape());
  BOOST_TEST(out0.container() == expected0);
  BOOST_TEST(out1.shape() == raster.shape());
  BOOST_TEST(out1.container() == expected1);
}

BOOST_AUTO_TEST_CASE(sum3_dirichlet_test) {
  const auto raster = Raster<int>({3, 3}).range();
  const auto in = extrapolate<NearestNeighbor>(raster);
  const auto kernel0 = OrientedKernel<int, 0>({1, 1, 1});
  const auto kernel1 = OrientedKernel<int, 1>({1, 1, 1});
  const auto out0 = kernel0 * in;
  const auto out1 = kernel1 * in;
  // 0 1 2 //  1  3  5 // 3  6  9
  // 3 4 5 // 10 12 14 // 9 12 15
  // 6 7 8 // 19 21 23 // 15 18 21
  const std::vector<int> expected0 {1, 3, 5, 10, 12, 14, 19, 21, 23};
  const std::vector<int> expected1 {3, 6, 9, 9, 12, 15, 15, 18, 21};
  BOOST_TEST(out0.shape() == raster.shape());
  BOOST_TEST(out0.container() == expected0);
  BOOST_TEST(out1.shape() == raster.shape());
  BOOST_TEST(out1.container() == expected1);
}

BOOST_AUTO_TEST_CASE(grad2_neumann_test) {
  const auto raster = Raster<int>({3, 3}).range();
  const auto in = extrapolate(raster, 0);
  const auto kernel0 = OrientedKernel<int, 0>({-1, 1});
  const auto kernel1 = OrientedKernel<int, 1>({-1, 1});
  const auto out0 = kernel0 * in;
  const auto out1 = kernel1 * in;
  // 0 1 2 // 1 1 -2 //  3  3  3
  // 3 4 5 // 1 1 -5 //  3  3  3
  // 6 7 8 // 1 1 -8 // -6 -7 -8
  const std::vector<int> expected0 {1, 1, -2, 1, 1, -5, 1, 1, -8};
  const std::vector<int> expected1 {3, 3, 3, 3, 3, 3, -6, -7, -8};
  BOOST_TEST(out0.shape() == raster.shape());
  BOOST_TEST(out0.container() == expected0);
  BOOST_TEST(out1.shape() == raster.shape());
  BOOST_TEST(out1.container() == expected1);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
