// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlTransforms/Kernel.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Kernel_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(coverage_test) {
  auto in = Raster<int>({4, 3}).fill(1);
  auto k = kernelize(Raster<int>({3, 3}).fill(1));
  auto out = k * extrapolate(in, 0);
  std::vector<int> expected = {4, 6, 6, 4, 6, 9, 9, 6, 4, 6, 6, 4};
  for (std::size_t i = 0; i < 12; ++i) {
    BOOST_TEST(out[i] == expected[i]);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
