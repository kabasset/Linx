// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RasterFourier/Dft.h"

#include <boost/test/unit_test.hpp>

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(RasterDemoDft_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(verbose_derivative_test) {

  //! [Plan direct]
  Cnes::RealDft<1> direct({360});
  //! [Plan direct]

  //! [Plan inverse]
  auto inverse = direct.inverse();
  // The type of inverse is RealDft<1>::Inverse.
  // inverse.in() is direct.out() and inverse.out() is direct.in()!
  //! [Plan inverse]

  //! [Fill signal]
  const double dx = Cnes::pi<double>() / 180;
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
  Cnes::Index k = 0;
  for (auto& e : direct.out()) {
    e *= std::complex<double>(0, k);
    ++k;
  };
  //! [Derivation]

  //! [Inverse transform]
  inverse.transform().normalize();
  // Inverse transformed data is in direct.in(); direct.out() is garbage now.
  //! [Inverse transform]

  for (Cnes::Index i = 0; i < direct.in().size(); ++i) {
    BOOST_TEST(std::abs(direct.in()[i] - std::cos(dx * i)) < .001);
  }
}

BOOST_AUTO_TEST_CASE(concise_derivative_test) {

  //! [Fill vector]
  Cnes::Raster<double, 1> in({360});
  const double dx = Cnes::pi<double>() / 180;
  double x = 0;
  for (auto& e : in) {
    e = std::sin(x);
    x += dx;
  }
  //! [Fill vector]

  //! [realDft]
  auto out = Cnes::realDft(in);
  //! [realDft]

  //! [Concise derivation]
  Cnes::Index k = 0;
  for (auto& e : out) {
    e *= std::complex<double>(0, k);
    ++k;
  };
  //! [Concise derivation]

  //! [inverseRealDft]
  auto inv = Cnes::inverseRealDft(out, {in.size()});
  //! [inverseRealDft]

  for (Cnes::Index i = 0; i < inv.size(); ++i) {
    BOOST_TEST(std::abs(inv[i] - std::cos(dx * i)) < .001);
  }
}

BOOST_AUTO_TEST_CASE(multithread_test) {
  // FIXME
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
