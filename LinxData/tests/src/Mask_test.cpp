// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Data/Mask.h"
#include "Linx/Data/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Mask_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(is_region_test)
{
  BOOST_TEST(is_region<Mask<1>>());
  BOOST_TEST(is_region<Mask<2>>());
  BOOST_TEST(is_region<Mask<3>>());
  BOOST_TEST(is_region<const Mask<1>>());
  BOOST_TEST(is_region<const Mask<2>>());
  BOOST_TEST(is_region<const Mask<3>>());
  BOOST_TEST(is_region<const Mask<1>&>());
  BOOST_TEST(is_region<const Mask<2>&>());
  BOOST_TEST(is_region<const Mask<3>&>());
}

template <Index P>
void check_ball(double radius = 2)
{
  const auto center = Position<3>::one();
  const auto ball = Mask<3>::ball<P>(radius, center);
  std::vector<Position<3>> out;
  std::vector<Position<3>> expected;
  BOOST_TEST(ball.box() == Box<3>::from_center(radius, center));
  for (const auto& p : ball.box()) {
    if (distance<P>(p, center) <= std::pow(radius, P)) {
      expected.push_back(p);
    }
  }
  for (const auto& p : ball) {
    out.push_back(p);
  }
  BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(l0_ball_test)
{
  check_ball<0>();
}

BOOST_AUTO_TEST_CASE(l1_ball_test)
{
  check_ball<1>();
}

BOOST_AUTO_TEST_CASE(l2_ball_test)
{
  check_ball<2>();
}

BOOST_AUTO_TEST_CASE(empty_mask_iterator_passes_test)
{
  Box<2> box {Position<2>::zero(), Position<2>::one()};
  Mask<2> mask(box, false);
  BOOST_CHECK(box.size() > 0);
  BOOST_CHECK(mask.size() == 0);
  for (const auto& p : mask) {
    std::cout << p << std::endl;
    throw std::out_of_range("We should not be there!");
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
