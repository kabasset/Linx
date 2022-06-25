// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Raster.h"
#include "LitlTransforms/MedianFilter.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(MedianFilter_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(constant0_3x3_test) {
  Raster<int, 2> in({4, 3});
  in.fill(1);
  MedianFilter<int, 2> filter;
  const auto extra = extrapolate(in, 0);
  auto out = filter.apply(extra);
  std::vector<int> expected {0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0};
  BOOST_TEST(out.container() == expected);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
