// Copyright (C) 2022, Antoine Basset
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Raster.h"

#include <boost/test/unit_test.hpp>

struct Fixture {

  Fixture() : a({3, 2}), b({3, 2}), c({3, 2}), k(2.) {
    a.range(1);
    b.fill(-1);
  }

  Litl::Raster<double> a;
  Litl::Raster<double> b;
  Litl::Raster<std::complex<double>> c;
  double k;
};

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(LitlDemoChannels_test, Fixture)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(operators_test) {

  //! [Operators]
  a += b; // Is equivalent to:
  std::transform(a.begin(), a.end(), b.begin(), a.begin(), [](auto e, auto f) {
    return e + f;
  });

  a += k; // Is equivalent to:
  std::transform(a.begin(), a.end(), a.begin(), [=](auto e) {
    return e + k;
  });

  auto res1 = a + b; // Is equivalent to:
  auto res2 = a;
  res2 += b;

  auto res3 = a + k; // Is equivalent to:
  auto res4 = a;
  res4 += k;
  //! [Operators]
}

BOOST_AUTO_TEST_CASE(functions_test) {

  //! [Functions]
  // Unary function
  auto res1 = cos(a); // Calls Litl::cos(a) according to ADL
  a.cos();

  // Binary function with scalar second argument
  auto res2 = pow(a, k);
  a.pow(k);

  // Binary function with raster second argument
  auto res3 = pow(a, b);
  a.pow(b);

  auto res4 = norm(c);
  // No in-place version when types do not match:
  // c is complex while res4 is real
  //! [Functions]
  std::cout << "cos(a) = " << res1[0] << std::endl;
  std::cout << "pow(a, k) = " << res2[0] << std::endl;
  std::cout << "pow(a, b) = " << res3[0] << std::endl;
  std::cout << "norm(c) = " << res4[0] << std::endl;
}

BOOST_AUTO_TEST_CASE(apply_test) {

  //! [Formula]
  auto res = a * k + b;
  //! [Formula]

  //! [Generate]
  res.generate(
      [=](auto e, auto f) {
        return e * k + f;
      },
      a,
      b);
  //! [Generate]

  //! [Apply]
  a.apply(
      [=](auto e, auto f) {
        return e * k + f;
      },
      b);
  //! [Apply]
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
