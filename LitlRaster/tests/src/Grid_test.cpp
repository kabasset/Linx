// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Grid.h"
#include "LitlRaster/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Grid_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(grid_is_screened_in_order_test) {
  Grid<2> region(Box<2>({1, 2}, {6, 7}), {2, 3});
  std::vector<Position<2>> expected {{1, 2}, {3, 2}, {5, 2}, {1, 5}, {3, 5}, {5, 5}};
  std::vector<Position<2>> out;
  for (const auto& p : region) {
    out.push_back(p);
  }
  BOOST_TEST(out == expected);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
