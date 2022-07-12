// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Subraster_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(singleton_subraster_test) {
  const Position<3> shape {3, 4, 5};
  const auto raster = random<float>(shape);
  const Position<3> pos {1, 2, 3};
  const Box<3> region {pos, pos};
  const auto subraster = raster.subraster(region);
  BOOST_TEST(subraster.size() == 1);
  BOOST_TEST((subraster[{0, 0, 0}] == raster[pos]));
}

BOOST_AUTO_TEST_CASE(domain_subraster_test) {
  const Position<3> shape {3, 4, 5};
  const auto raster = random<float>(shape);
  const auto region = raster.domain();
  const auto subraster = raster.subraster(region);
  BOOST_TEST(subraster.shape() == shape);
  for (Position<3> p : region) {
    BOOST_TEST(subraster[p] == raster[p]);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
