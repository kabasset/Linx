// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Base/TypeUtils.h"
#include "Linx/Data/Sequence.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Sequence_test)

//-----------------------------------------------------------------------------

LINX_TEST_CASE_TEMPLATE(element_access_test)
{
  T data[] = {T(0), T(1), T(0), T(1), T(2), T(3)};
  Sequence<T> seq(6, data);
  BOOST_TEST(seq.size() == 6);
  for (std::size_t i = 0; i < 6; ++i) {
    BOOST_TEST(seq[i] == data[i]);
  }
  seq[0] = T(1);
  BOOST_TEST(seq[0] == T(1));
}

BOOST_AUTO_TEST_CASE(arithmetic_test)
{
  const auto lhs = random<int>(314);
  const auto rhs = random<int>(314);
  int scalar = 2;
  const auto plus_vector = lhs + rhs;
  const auto plus_scalar = lhs + scalar;
  const auto minus_vector = lhs - rhs;
  const auto minus_scalar = lhs - scalar;
  const auto times_vector = lhs * rhs;
  const auto times_scalar = lhs * scalar;
  const auto divided_by_vector = lhs / rhs;
  const auto divided_by_scalar = lhs / scalar;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    BOOST_TEST(plus_vector[i] == lhs[i] + rhs[i]);
    BOOST_TEST(plus_scalar[i] == lhs[i] + scalar);
    BOOST_TEST(minus_vector[i] == lhs[i] - rhs[i]);
    BOOST_TEST(minus_scalar[i] == lhs[i] - scalar);
    BOOST_TEST(times_vector[i] == lhs[i] * rhs[i]);
    BOOST_TEST(times_scalar[i] == lhs[i] * scalar);
    BOOST_TEST(divided_by_vector[i] == lhs[i] / rhs[i]);
    BOOST_TEST(divided_by_scalar[i] == lhs[i] / scalar);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
