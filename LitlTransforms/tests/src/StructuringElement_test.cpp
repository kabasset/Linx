// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Raster.h"
#include "LitlTransforms/StructuringElement.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(StructuringElement_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(constant0_3x3_test) {
  Raster<int, 2> in({4, 3});
  in.fill(1);
  StructuringElement<Box<2>> filter;
  const auto extra = extrapolate(in, 0);

  auto medianOut = filter.median(extra);
  std::vector<int> medianExpected {0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0};
  BOOST_TEST(medianOut.container() == medianExpected);

  auto erodeOut = filter.erode(extra);
  std::vector<int> erodeExpected {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0};
  BOOST_TEST(erodeOut.container() == erodeExpected);

  auto dilateOut = filter.dilate(extra);
  std::vector<int> dilateExpected {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  BOOST_TEST(dilateOut.container() == dilateExpected);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
