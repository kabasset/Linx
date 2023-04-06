// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LinxBase/TypeUtils.h"
#include "LinxCore/Sequence.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Sequence_test)

//-----------------------------------------------------------------------------

LINX_TEST_CASE_TEMPLATE(element_access_test) {
  T data[] = {T(0), T(1), T(0), T(1), T(2), T(3)};
  Sequence<T> seq(6, data);
  BOOST_TEST(seq.size() == 6);
  for (std::size_t i = 0; i < 6; ++i) {
    BOOST_TEST(seq[i] == data[i]);
  }
  seq[0] = T(1);
  BOOST_TEST(seq[0] == T(1));
}

BOOST_AUTO_TEST_CASE(arithmetic_test) {
  const auto lhs = random<int>(314);
  const auto rhs = random<int>(314);
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

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
