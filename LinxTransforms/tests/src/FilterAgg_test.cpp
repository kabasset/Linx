// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Transforms/Filters.h"
#include "Linx/Transforms/Interpolation.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(FilterAgg_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(laplacian_plus_test)
{
  Raster<int> expected({3, 3}, {0, 1, 0, 1, -4, 1, 0, 1, 0});
  BOOST_TEST((laplace_operator<int, 0, 1>().impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(laplacian_minus_test)
{
  Raster<int> expected({3, 3}, {0, -1, 0, -1, 4, -1, 0, -1, 0});
  BOOST_TEST((laplace_operator<int, 0, 1>(-1).impulse()) == expected);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
