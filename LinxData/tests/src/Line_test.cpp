// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Data/Line.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Line_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(is_region_test)
{
  BOOST_TEST(is_region<Line<0, 1>>());
  BOOST_TEST(is_region<Line<0, 2>>());
  BOOST_TEST(is_region<Line<0, 3>>());
  BOOST_TEST(is_region<const Line<0, 1>>());
  BOOST_TEST(is_region<const Line<0, 2>>());
  BOOST_TEST(is_region<const Line<0, 3>>());
  BOOST_TEST(is_region<const Line<0, 1>&>());
  BOOST_TEST(is_region<const Line<0, 2>&>());
  BOOST_TEST(is_region<const Line<0, 3>&>());
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
