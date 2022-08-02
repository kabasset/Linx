// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Box.h"
#include "LitlRaster/Grid.h"
#include "LitlRaster/Mask.h"
#include "LitlRaster/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(SubrasterIterator_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(inner_cube_iterator_test) {

  // Setup
  const Position<3> shape {4, 5, 6};
  const auto raster = Raster<float, 3>(shape).range();
  const Box<3> region {{1, 1, 1}, {2, 3, 4}};

  // Expected
  std::vector<float> expected;
  expected.reserve(region.size());
  for (const auto& p : region) {
    expected.push_back(raster[p]);
  }

  // Box
  std::vector<float> out;
  out.reserve(region.size());
  const auto boxed = raster.subraster(region);
  for (const auto& v : boxed) {
    out.push_back(v);
  }
  BOOST_TEST(out == expected);

  // Grid
  out.clear();
  const auto grided = raster.subraster(Grid<3>(region, Position<3>::one()));
  for (const auto& v : grided) {
    out.push_back(v);
  }
  BOOST_TEST(out == expected);

  // Mask
  out.clear();
  const auto masked = raster.subraster(Mask<3>(region, true));
  for (const auto& v : masked) {
    out.push_back(v);
  }
  BOOST_TEST(out == expected);

  // Sequence
  out.clear();
  const Sequence<Position<3>> seq(region);
  const auto sequenced = raster.subraster(seq);
  for (const auto& v : sequenced) {
    out.push_back(v);
  }
  // BOOST_TEST(out == expected);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
