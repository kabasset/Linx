// Copyright (C) 2022, Antoine Basset
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RasterGeometry/Interpolation.h"

#include <boost/test/unit_test.hpp>

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(LitlDemoInterpolation_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(extrapolate_interpolate_test) {

  //! [Unit raster]
  Litl::Raster<int> raster({2, 2});
  raster.range(1);
  //! [Unit raster]

  //! [Extra]
  Litl::Position<2> outside {-1, -1};
  const auto dirichlet = Litl::extrapolate(raster, 0);
  const auto neumann = Litl::extrapolate<Litl::NearestNeighbor>(raster);
  BOOST_TEST(dirichlet[outside] == 0);
  BOOST_TEST(neumann[outside] == raster[0]);
  //! [Extra]

  //! [Inter]
  Litl::Vector<double, 2> inbetween {.5, .5};
  const auto nn = Litl::interpolate<Litl::NearestNeighbor>(raster);
  BOOST_TEST(nn.at(inbetween) == raster.at(-1));
  //! [Inter]

  //! [Inter-extra]
  const auto linearDirichlet = Litl::interpolate<Litl::Linear>(dirichlet);
  BOOST_TEST(linearDirichlet[outside] == 0);
  BOOST_TEST(linearDirichlet.at(inbetween) == 2.5);
  //! [Inter-extra]
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
