// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE VectorArithmeticsTest

#include "Linx/Data/Vector.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(dynamic_rank_test)
{
  using namespace Linx::Literals;

  auto u = Linx::vec({1, 2, 3});
  auto v = Linx::vec({1, 1, 1, 1}); // FIXME vec(4_D, 1)
  auto w = u + v;

  BOOST_TEST(w.n == -1);
  BOOST_TEST(w.size() == 4);
  BOOST_TEST(w(0) == 2);
  BOOST_TEST(w(1) == 3);
  BOOST_TEST(w(2) == 4);
  BOOST_TEST(w(3) == 1);
}

BOOST_AUTO_TEST_CASE(static_rank_test)
{
  using namespace Linx::Literals;

  auto u = Linx::vec(1, 2, 3);
  auto v = Linx::vec<4_D>(1);
  auto w = u + v;

  BOOST_TEST(w.n == 4);
  BOOST_TEST(w(0) == 2);
  BOOST_TEST(w(1) == 3);
  BOOST_TEST(w(2) == 4);
  BOOST_TEST(w(3) == 1);
}

BOOST_AUTO_TEST_CASE(static_coefs_test)
{
  using namespace Linx::Literals;

  auto u = Linx::vec<1, 2, 3>();
  auto v = Linx::vec<4_D, 1>();
  auto w = u + v;

  BOOST_TEST(w.n == 4);
  BOOST_TEST(w(0) == 2);
  BOOST_TEST(w(1) == 3);
  BOOST_TEST(w(2) == 4);
  BOOST_TEST(w(3) == 1);
}

BOOST_AUTO_TEST_CASE(zero_test)
{
  auto u = Linx::vec<3, 2, 1>();
  auto v = Linx::vec();
  auto w = u + v;

  BOOST_TEST(w.n == 3);
  BOOST_TEST(w(0) == 3);
  BOOST_TEST(w(1) == 2);
  BOOST_TEST(w(2) == 1);

  auto x = v + u;

  BOOST_TEST(x.n == 3);
  BOOST_TEST(x(0) == 3);
  BOOST_TEST(x(1) == 2);
  BOOST_TEST(x(2) == 1);
}

BOOST_AUTO_TEST_SUITE_END()
