// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Data/Box.h"

#include <boost/test/unit_test.hpp>
#include <set>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Box_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(is_region_test)
{
  BOOST_TEST(is_region<Box<1>>());
  BOOST_TEST(is_region<Box<2>>());
  BOOST_TEST(is_region<Box<3>>());
  BOOST_TEST(is_region<const Box<1>>());
  BOOST_TEST(is_region<const Box<2>>());
  BOOST_TEST(is_region<const Box<3>>());
  BOOST_TEST(is_region<const Box<1>&>());
  BOOST_TEST(is_region<const Box<2>&>());
  BOOST_TEST(is_region<const Box<3>&>());
}

BOOST_AUTO_TEST_CASE(ctors_test)
{
  Position<7> front;
  ++front;
  Position<7> back {2, 8, 6, 1, 9, 8, 9};
  Box<7> from_to {front, back};
  BOOST_TEST(from_to.shape() == back);
  Box<7> from_shape {front, from_to.shape()};
  BOOST_TEST(from_shape.back() == back);
}

BOOST_AUTO_TEST_CASE(translation_test)
{
  const Position<3> front {1, 2, 3};
  const Position<3> back {4, 5, 6};
  Box<3> region {front, back};
  const auto shape = region.shape();
  region += shape - 1;
  BOOST_TEST(region.shape() == shape);
  BOOST_TEST(region.front() == back);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
