// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(SubrasterIterator_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(inner_cube_iterator_test) {

  const Position<3> shape {4, 5, 6};
  const auto raster = Raster<float, 3>(shape).range();
  const Box<3> region {{1, 1, 1}, {2, 3, 4}};
  const auto subraster = raster.subraster(region);

  std::vector<float> expected;
  expected.reserve(region.size());
  for (const auto& p : region) {
    expected.push_back(raster[p]);
  }

  std::vector<float> out;
  out.reserve(region.size());
  for (const auto& v : subraster) {
    out.push_back(v);
  }

  BOOST_TEST(out == expected);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
