// Copyright (C) 2022, Antoine Basset
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Raster.h"

#include <boost/test/unit_test.hpp>

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(LitlDemoRandom_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(simple_random_test) {

  //! [Generate Gaussian noise]
  Litl::Raster<double> raster({3, 2});
  raster.generate(Litl::GaussianNoise<double>(100, 15));
  //! [Generate Gaussian noise]

  std::cout << "Random raster: " << raster << std::endl;

  //! [Apply Poisson noise]
  raster.apply(Litl::StablePoissonNoise<int>());
  //! [Apply Poisson noise]

  std::cout << "Noisy raster:  " << raster << std::endl;

  //! [Apply salt and pepper noise]
  raster.apply(Litl::ImpulseNoise<int>::saltAndPepper(0.1, 0.2, 0, 1000));
  //! [Apply salt and pepper noise]

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
  Litl::PoissonNoise<T> m_poisson;
  Litl::GaussianNoise<T> m_gaussian;
};
//! [System noise]

BOOST_AUTO_TEST_CASE(compound_random_test) {

  Litl::Raster<double> flux({3, 2});
  Litl::Raster<double> dark({3, 2});
  flux.generate(Litl::PoissonNoise<int>(100));
  dark.generate(Litl::UniformNoise<double>(0, 10));
  std::cout << "Flux: " << flux << std::endl;
  std::cout << "Dark: " << dark << std::endl;

  //! [Apply system noise]
  // New instance
  Litl::Raster<double> res(flux);
  res.generate(SystemNoise<double>(), flux, dark);

  // In-place
  flux.apply(SystemNoise<double>(), dark);
  //! [Apply system noise]

  std::cout << "From generate: " << res << std::endl;
  std::cout << "From apply: " << flux << std::endl;
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
