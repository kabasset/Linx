// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Box.h"
#include "LitlRaster/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(BoxIterator_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(domain_is_screened_in_order_test) {
  Position<5> shape {2, 3, 4, 5, 6};
  Raster<Index, 5> raster(shape);
  for (std::size_t i = 0; i < raster.size(); ++i) {
    raster[i] = i;
  }
  Index i = 0;
  for (const auto& p : raster.domain()) {
    BOOST_TEST(raster.index(p) == i);
    ++i;
  }
}

BOOST_AUTO_TEST_CASE(box_is_screened_in_order_test) {
  Position<4> shape {3, 4, 5, 6};
  Raster<Index, 4> raster(shape);
  Box<4> region {Position<4>::zero() + 1, shape - 2};
  for (std::size_t i = 0; i < raster.size(); ++i) {
    raster[i] = i;
  }
  Index current = 0;
  Index count = 0;
  for (const auto& p : region) {
    BOOST_TEST(raster.index(p) > current);
    current = raster.index(p);
    ++count;
  }
  BOOST_TEST(count == region.size());
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
