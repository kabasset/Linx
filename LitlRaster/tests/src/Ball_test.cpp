// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Ball.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Ball_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(properties_test) {
  const double radius = 2.3;
  const Index length = 2 * Index(radius) + 1;
  const Position<3> center {1, 2, 3};
  const auto box = Box<3>::fromCenter(radius, center);
  Ball<3> ball(radius, center);
  BOOST_TEST(ball.radius() == radius);
  BOOST_TEST(ball.center() == center);
  BOOST_TEST(ball.box().front() == box.front());
  BOOST_TEST(ball.box().back() == box.back());
  BOOST_TEST(ball.template length<0>() == length);
  BOOST_TEST(ball.length(0) == length);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
