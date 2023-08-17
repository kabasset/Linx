// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Transforms/Extrapolation.h" // FIXME own test suite
#include "Linx/Transforms/Interpolation.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Interpolation_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(constant_nn_test)
{
  Raster<int, 3> raster({2, 2, 2});
  raster.fill(1);

  Position<3> position {-1, -1, -1};
  Vector<double, 3> vector {.5, .5, .5};

  const auto extra = extrapolate(raster, 0);
  BOOST_TEST(extra[position] == 0);

  const auto inter = interpolate<NearestNeighbor>(raster);
  BOOST_TEST(&inter[position] < raster.data()); // Out of bounds
  BOOST_TEST(inter(vector) == 1);

  const auto interextra = interpolate<NearestNeighbor>(extra);
  BOOST_TEST(interextra[position] == 0);
  BOOST_TEST(interextra(vector) == 1);
}

BOOST_AUTO_TEST_CASE(periodic_test)
{
  Raster<int, 3> raster({2, 2, 2});
  raster.range(1);

  Position<3> negative {-1, -1, -1};
  Position<3> positive {2, 3, 4};

  const auto extra = extrapolate<Periodic>(raster);
  BOOST_TEST(extra[negative] == raster.at(-1));
  BOOST_TEST(extra[positive] == (raster[{0, 1, 0}]));
}

BOOST_AUTO_TEST_CASE(linear_test)
{
  Raster<int, 3> raster({2, 2, 2});
  raster.range(1);

  const auto interpolator = interpolate<Linear>(raster);

  const auto front = interpolator[{0, 0, 0}];
  const auto back = interpolator[{1, 1, 1}];
  const auto center = interpolator({.5, .5, .5});

  BOOST_TEST(front == 1);
  BOOST_TEST(back == 8);
  BOOST_TEST(center == 4.5);
}

BOOST_AUTO_TEST_CASE(cubic_test)
{
  Raster<int, 3> raster({4, 4, 4});
  raster.range(1);

  const auto interpolator = interpolate<Cubic>(raster);

  const auto front = interpolator[{0, 0, 0}];
  const auto back = interpolator[{3, 3, 3}];
  const auto pos = interpolator[{1, 1, 1}];
  const auto center = interpolator({1.5, 1.5, 1.5});

  BOOST_TEST(front == 1);
  BOOST_TEST(back == 64);
  BOOST_TEST(pos == 22);
  BOOST_TEST(center == 32.5);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
