// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Raster/Arithmetic.h"
#include "Raster/TestRaster.h"

#include <boost/test/unit_test.hpp>

using namespace Cnes;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Arithmetic_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(raster_arithmetic_test) {
  const Position<2> shape {19, 6};
  const Test::RandomRaster<int> lhs(shape);
  const Test::RandomRaster<int> rhs(shape);
  int scalar = 2;
  const auto plusVector = lhs + rhs;
  const auto plusScalar = lhs + scalar;
  const auto minusVector = lhs - rhs;
  const auto minusScalar = lhs - scalar;
  const auto timesVector = lhs * rhs;
  const auto timesScalar = lhs * scalar;
  const auto dividedByVector = lhs / rhs;
  const auto dividedByScalar = lhs / scalar;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    BOOST_TEST(plusVector[i] == lhs[i] + rhs[i]);
    BOOST_TEST(plusScalar[i] == lhs[i] + scalar);
    BOOST_TEST(minusVector[i] == lhs[i] - rhs[i]);
    BOOST_TEST(minusScalar[i] == lhs[i] - scalar);
    BOOST_TEST(timesVector[i] == lhs[i] * rhs[i]);
    BOOST_TEST(timesScalar[i] == lhs[i] * scalar);
    BOOST_TEST(dividedByVector[i] == lhs[i] / rhs[i]);
    BOOST_TEST(dividedByScalar[i] == lhs[i] / scalar);
  }
}

BOOST_AUTO_TEST_CASE(raster_generate_test) {
  Position<3> shape {3, 14, 15};
  Test::RandomRaster<std::int16_t, 3> a(shape);
  Test::RandomRaster<std::int32_t, 3> b(shape);
  VecRaster<std::int64_t, 3> result(shape);
  result.generate(
      [](auto v, auto w) {
        return v * w;
      },
      a,
      b);
  result.apply([](auto v) {
    return -v;
  });
  for (const auto& p : result.domain()) {
    BOOST_TEST((result[p] == -a[p] * b[p]));
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
