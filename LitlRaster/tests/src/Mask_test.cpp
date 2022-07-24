// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Mask.h"
#include "LitlRaster/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Mask_test)

//-----------------------------------------------------------------------------

template <Index P>
void checkBall(double radius = 2) {
  const auto ball = Mask<3>::ball<P>(radius);
  std::vector<Position<3>> out;
  std::vector<Position<3>> expected;
  for (const auto& p : ball.box()) {
    if (p.template norm<P>() <= std::pow(radius, P)) {
      expected.push_back(p);
    }
  }
  for (const auto& p : ball) {
    out.push_back(p);
  }
  BOOST_TEST(out == expected);
  BOOST_TEST(ball.domain().container() == expected);
}

BOOST_AUTO_TEST_CASE(l0_ball_test) {
  checkBall<0>();
}

BOOST_AUTO_TEST_CASE(l1_ball_test) {
  checkBall<1>();
}

BOOST_AUTO_TEST_CASE(l2_ball_test) {
  checkBall<2>();
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
