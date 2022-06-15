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

  Extrapolator<OutOfBoundsConstant<int>, int, 3> extra(OutOfBoundsConstant<int>(0), raster);
  BOOST_TEST(extra[position] == 0);

  Interpolator<NearestNeighbor, decltype(raster)> inter(NearestNeighbor(), raster);
  BOOST_TEST(inter[vector] == 1);

  Interpolator<NearestNeighbor, decltype(extra)> inextra(NearestNeighbor(), extra);
  BOOST_TEST(inextra[position] == 0);
  BOOST_TEST(inextra[vector] == 1);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
