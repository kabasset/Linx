// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RasterFourier/Dft.h"

#include <boost/test/unit_test.hpp>

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(LitlDemoDft_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(verbose_derivative_test) {

  //! [Plan direct]
  Litl::RealDft<1> direct({360});
  //! [Plan direct]

  //! [Plan inverse]
  auto inverse = direct.inverse();
  // The type of inverse is RealDft<1>::Inverse.
  // inverse.in() is direct.out() and inverse.out() is direct.in()!
  //! [Plan inverse]

  //! [Fill signal]
  const double dx = Litl::pi<double>() / 180;
  double x = 0;
  for (auto& e : direct.in()) {
    e = std::sin(x);
    x += dx;
  }
  //! [Fill signal]

  //! [Direct transform]
  direct.transform();
  // Transformed data is in direct.out(); direct.in() is garbage now.
  //! [Direct transform]

  //! [Derivation]
  Litl::Index k = 0;
  for (auto& e : direct.out()) {
    e *= std::complex<double>(0, k);
    ++k;
  };
  //! [Derivation]

  //! [Inverse transform]
  inverse.transform().normalize();
  // Inverse transformed data is in direct.in(); direct.out() is garbage now.
  //! [Inverse transform]

  for (std::size_t i = 0; i < direct.in().size(); ++i) {
    BOOST_TEST(std::abs(direct.in()[i] - std::cos(dx * i)) < .001);
  }
}

BOOST_AUTO_TEST_CASE(concise_derivative_test) {

  //! [Fill vector]
  Litl::Index size = 360;
  Litl::Raster<double, 1> in({size});
  const double dx = Litl::pi<double>() / 180;
  double x = 0;
  for (auto& e : in) {
    e = std::sin(x);
    x += dx;
  }
  //! [Fill vector]

  //! [realDft]
  auto out = Litl::realDft(in);
  //! [realDft]

  //! [Concise derivation]
  Litl::Index k = 0;
  for (auto& e : out) {
    e *= std::complex<double>(0, k);
    ++k;
  };
  //! [Concise derivation]

  //! [inverseRealDft]
  auto inv = Litl::inverseRealDft(out, {size});
  //! [inverseRealDft]

  for (Litl::Index i = 0; i < size; ++i) {
    BOOST_TEST(std::abs(inv[i] - std::cos(dx * i)) < .001);
  }
}

BOOST_AUTO_TEST_CASE(multithread_test) {
  // FIXME
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
