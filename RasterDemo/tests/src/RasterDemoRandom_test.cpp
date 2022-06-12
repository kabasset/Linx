// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Raster/Random.h"
#include "Raster/Raster.h"

#include <boost/test/unit_test.hpp>

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(RasterDemoRandom_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(simple_random_test) {

  //! [Random values]
  Cnes::Raster<double> raster({3, 2});
  raster.generate(Cnes::GaussianNoise<double>(100, 15));
  //! [Random values]

  std::cout << "Random raster: " << raster << std::endl;

  //! [Random noise]
  raster.apply(Cnes::StablePoissonNoise<double>());
  //! [Random noise]

  std::cout << "Noisy raster:  " << raster << std::endl;

  //! [SnP noise]
  raster.apply(Cnes::ImpulseNoise<int>::saltAndPepper(0.1, 0.2, 0, 1000));
  //! [SnP noise]

  std::cout << "S&P noised: " << raster << std::endl;
}

//! [System noise]
template <typename T>
class SystemNoise {

public:
  explicit SystemNoise(std::size_t seed = 0) : m_poisson(0, seed), m_gaussian(0, 1, seed) {}

  T operator()(T flux, T dark) {
    auto out = m_poisson(flux);
    out += dark + std::sqrt(dark) * m_gaussian();
    return out;
  }

private:
  Cnes::PoissonNoise<T> m_poisson;
  Cnes::GaussianNoise<T> m_gaussian;
};
//! [System noise]

BOOST_AUTO_TEST_CASE(compound_random_test) {

  Cnes::Raster<double> flux({3, 2});
  Cnes::Raster<double> dark({3, 2});
  flux.generate(Cnes::PoissonNoise<int>(100));
  dark.generate(Cnes::UniformNoise<double>(0, 10));
  std::cout << "Flux: " << flux << std::endl;
  std::cout << "Dark: " << dark << std::endl;

  //! [Apply system noise]
  // New instance
  Cnes::Raster<double> res(flux);
  res.generate(SystemNoise<double>(), flux, dark);

  // In-place
  flux.apply(SystemNoise<double>(), dark);
  //! [Apply system noise]

  std::cout << "From generate: " << res << std::endl;
  std::cout << "From apply: " << flux << std::endl;
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
