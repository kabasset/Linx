// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Transforms/FilterSeq.h"
#include "Linx/Transforms/Interpolation.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(FilterSeq_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(prewitt_inc_test)
{
  Raster<int> expected({3, 3}, {1, 0, -1, 1, 0, -1, 1, 0, -1});
  BOOST_TEST((prewitt_filter<int, 0, 1>().impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(prewitt_dec_test)
{
  Raster<int> expected({3, 3}, {-1, 0, 1, -1, 0, 1, -1, 0, 1});
  BOOST_TEST((prewitt_filter<int, 0, 1>(-1).impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(sobel_inc_test)
{
  Raster<int> expected({3, 3}, {1, 0, -1, 2, 0, -2, 1, 0, -1});
  BOOST_TEST((sobel_filter<int, 0, 1>().impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(sobel_dec_test)
{
  Raster<int> expected({3, 3}, {-1, 0, 1, -2, 0, 2, -1, 0, 1});
  BOOST_TEST((sobel_filter<int, 0, 1>(-1).impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(scharr_inc_test)
{
  Raster<int> expected({3, 3}, {3, 0, -3, 10, 0, -10, 3, 0, -3});
  BOOST_TEST((scharr_filter<int, 0, 1>().impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(scharr_dec_test)
{
  Raster<int> expected({3, 3}, {-3, 0, 3, -10, 0, 10, -3, 0, 3});
  BOOST_TEST((scharr_filter<int, 0, 1>(-1).impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(laplacian_plus_test)
{
  Raster<int> expected({3, 3}, {0, 1, 0, 1, -2, 1, 0, 1, 0});
  BOOST_TEST((laplacian_filter<int, 0, 1>().impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(laplacian_minus_test)
{
  Raster<int> expected({3, 3}, {0, -1, 0, -1, 2, -1, 0, -1, 0});
  BOOST_TEST((laplacian_filter<int, 0, 1>(-1).impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(orthogonal_associativity_commutativity_test)
{
  const auto a = correlation_along<int, 0>({1, 0, -1});
  const auto b = correlation_along<int, 1>({1, 2, 3});
  const auto c = a * b;
  const auto raster = Raster<int>({3, 3}).range();
  const auto direct = c * extrapolation(raster, 0);
  const auto associated = a * b * extrapolation(raster, 0);
  const auto commutated = b * a * extrapolation(raster, 0);
  BOOST_TEST(associated == direct);
  BOOST_TEST(commutated == direct);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
