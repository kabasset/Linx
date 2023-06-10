// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Base/Math.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Math_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(abspow_test) {
  BOOST_TEST(abspow<0>(0) == 0);
  BOOST_TEST(abspow<0>(2) == 1);
  BOOST_TEST(abspow<0>(-2) == 1);
  BOOST_TEST(abspow<1>(0) == 0);
  BOOST_TEST(abspow<1>(2) == 2);
  BOOST_TEST(abspow<1>(-2) == 2);
  BOOST_TEST(abspow<2>(0) == 0);
  BOOST_TEST(abspow<2>(2) == 4);
  BOOST_TEST(abspow<2>(-2) == 4);
  BOOST_TEST(abspow<3>(0) == 0);
  BOOST_TEST(abspow<3>(2) == 8);
  BOOST_TEST(abspow<3>(-2) == 8);
  BOOST_TEST(abspow<4>(0) == 0);
  BOOST_TEST(abspow<4>(2) == 16);
  BOOST_TEST(abspow<4>(-2) == 16);
}

BOOST_AUTO_TEST_CASE(FIXME_test) {

  BOOST_FAIL("!!!! Please implement your tests !!!!");
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
