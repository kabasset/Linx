// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Raster/Random.h"
#include "Raster/Raster.h"

#include <boost/test/unit_test.hpp>

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(RasterDemoRandom_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(random_test) {

  //! [Random values]
  Cnes::Raster<double> raster({3, 2});
  raster.generate(Cnes::GaussianNoise<double>(100, 15));
  //! [Random values]

  std::cout << "Random raster: " << raster << std::endl;

  //! [Random noise]
  raster.apply(Cnes::StablePoissonNoise<double>());
  //! [Random noise]

  std::cout << "Noisy raster: " << raster << std::endl;
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
