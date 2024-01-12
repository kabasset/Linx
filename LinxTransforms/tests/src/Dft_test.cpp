// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Transforms/Dft.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Dft_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(dft_r2c_c2c_ic2c_ir2c_test)
{
  constexpr Index width = 5;
  constexpr Index half_width = (width + 1) / 2;
  constexpr Index height = 6;
  const Position<2> shape {width, height};
  const Position<2> half_shape {half_width, height};

  // Make r2c
  RealDft<2> real({width, height});
  BOOST_TEST(real.in().shape() == shape);
  BOOST_TEST(real.in().owns());
  BOOST_TEST(real.in().data() != nullptr);
  BOOST_TEST(real.out().shape() == half_shape);
  BOOST_TEST(real.out().owns());
  BOOST_TEST(real.out().data() != nullptr);

  // Make ir2c
  auto inverse_real = real.inverse();
  BOOST_TEST(inverse_real.in().shape() == half_shape);
  BOOST_TEST(not inverse_real.in().owns());
  BOOST_TEST(inverse_real.in().data() == real.out().data());
  BOOST_TEST(inverse_real.out().shape() == shape);
  BOOST_TEST(not inverse_real.out().owns());
  BOOST_TEST(inverse_real.out().data() == real.in().data());

  // Make c2c
  auto complex = real.template compose<ComplexDft<2>>(real.out_shape());
  BOOST_TEST(complex.in().shape() == half_shape);
  BOOST_TEST(not complex.in().owns());
  BOOST_TEST(complex.in().data() == real.out().data());
  BOOST_TEST(complex.out().shape() == half_shape);
  BOOST_TEST(complex.out().owns());
  BOOST_TEST(complex.out().data() != nullptr);

  // Make ic2c
  auto inverse_complex = complex.inverse();
  BOOST_TEST(inverse_complex.in().shape() == half_shape);
  BOOST_TEST(not inverse_complex.in().owns());
  BOOST_TEST(inverse_complex.in().data() == complex.out().data());
  BOOST_TEST(inverse_complex.out().shape() == half_shape);
  BOOST_TEST(not inverse_complex.out().owns());
  BOOST_TEST(inverse_complex.out().data() == complex.in().data());

  // Fill r2c signal
  auto& signal = real.in();
  for (const auto& p : signal.domain()) {
    signal[p] = 1 + p[0] + p[1];
  }

  // Apply r2c and then c2c
  BOOST_TEST(real.in().data() != nullptr);
  BOOST_TEST(real.out().data() != nullptr);
  real.transform();
  BOOST_TEST(complex.in().data() != nullptr);
  BOOST_TEST(complex.out().data() != nullptr);
  complex.transform();

  // Apply ic2c and then ir2c
  BOOST_TEST(inverse_complex.in().data() != nullptr);
  BOOST_TEST(inverse_complex.out().data() != nullptr);
  inverse_complex.transform().normalize();
  BOOST_TEST(inverse_real.in().data() != nullptr);
  BOOST_TEST(inverse_real.out().data() != nullptr);
  inverse_real.transform().normalize();

  // Check values are recovered
  for (const auto& p : signal.domain()) {
    const auto expected = 1 + p[0] + p[1];
    const auto value = signal[p];
    BOOST_TEST(value > 0.99 * expected);
    BOOST_TEST(value < 1.01 * expected);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
