// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LinxCore/Tiling.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Tiling_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(region_tiling_test) {
  Box<3> region({3, 4, 5}, {9, 8, 7});
  const auto tiles = tileRegionAlong<1>(region);
  const auto plane = tiles.domain() + region.front();
  BOOST_TEST(plane == Box<3>({3, 4, 5}, {9, 4, 7}));
  for (const auto& p : tiles.domain()) {
    const auto line = tiles[p];
    BOOST_TEST(line.front() == p + region.front());
    BOOST_TEST(line.size() == 5);
  }
}

BOOST_AUTO_TEST_CASE(region_tiling_ordering_test) {
  Box<3> region({3, 4, 5}, {9, 8, 7});
  const auto tiles = tileRegionAlong<0>(region);
  const auto plane = tiles.domain() + region.front();
  BOOST_TEST(plane == Box<3>({3, 4, 5}, {3, 8, 7}));
  auto it = region.begin();
  for (const auto& p : tiles.domain()) {
    const auto line = tiles[p];
    for (const auto& q : line) {
      BOOST_TEST(q == *it);
      ++it;
    }
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
