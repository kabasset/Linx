// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RasterGeometry/Interpolation.h"

#include <boost/test/unit_test.hpp>

using namespace Cnes;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Interpolation_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(constant_nn_test) {
  Raster<int, 3> raster({2, 2, 2});
  raster.fill(1);

  Position<3> position {-1, -1, -1};
  Vector<double, 3> vector {.5, .5, .5};

  const auto extra = makeExtrapolator(OutOfBoundsConstant<int>(0), raster);
  BOOST_TEST(extra[position] == 0);

  const auto inter = makeInterpolator(NearestNeighbor(), raster);
  BOOST_TEST(&inter[position] < raster.data()); // Out of bounds
  BOOST_TEST(inter[vector] == 1);

  const auto inextra = makeInterpolator(NearestNeighbor(), extra);
  BOOST_TEST(inextra[position] == 0);
  BOOST_TEST(inextra[vector] == 1);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
