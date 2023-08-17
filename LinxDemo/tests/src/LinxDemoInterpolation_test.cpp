// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Transforms/Extrapolation.h"
#include "Linx/Transforms/Interpolation.h"

#include <boost/test/unit_test.hpp>

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(LinxDemoInterpolation_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(extrapolate_interpolate_test)
{
  //! [Unit raster]
  Linx::Raster<int> raster({2, 2});
  raster.range(1);
  //! [Unit raster]

  //! [Extra]
  Linx::Position<2> outside {-1, -1};
  const auto dirichlet = Linx::extrapolate(raster, 0);
  const auto neumann = Linx::extrapolate<Linx::NearestNeighbor>(raster);
  BOOST_TEST(dirichlet[outside] == 0);
  BOOST_TEST(neumann[outside] == raster[0]);
  //! [Extra]

  //! [Inter]
  Linx::Vector<double, 2> inbetween {.5, .5};
  const auto nn = Linx::interpolate<Linx::NearestNeighbor>(raster);
  BOOST_TEST(nn(inbetween) == raster.at(-1));
  //! [Inter]

  //! [Inter-extra]
  const auto linear_dirichlet = Linx::interpolate<Linx::Linear>(dirichlet);
  BOOST_TEST(linear_dirichlet[outside] == 0);
  BOOST_TEST(linear_dirichlet(inbetween) == 2.5);
  //! [Inter-extra]
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
