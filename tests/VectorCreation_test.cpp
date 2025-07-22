// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE VectorCreationTest

#include "Linx/Base/Vector.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(dynamic_rank_test)
{
  auto v = Linx::vec({1, 2, 3});
  BOOST_TEST(v.n == -1);
  BOOST_TEST(v.size() == 3);
  BOOST_TEST(v(0) == 1);
  BOOST_TEST(v(1) == 2);
  BOOST_TEST(v(2) == 3);
}

BOOST_AUTO_TEST_CASE(static_rank_test)
{
  auto v = Linx::vec(1, 2, 3);
  BOOST_TEST(v.n == 3);
  BOOST_TEST(v.size() == 3);
  BOOST_TEST(v(0) == 1);
  BOOST_TEST(v(1) == 2);
  BOOST_TEST(v(2) == 3);

  using namespace Linx::Literals;

  auto w = Linx::vec<3_D>(1);
  BOOST_TEST(w.n == 3);
  BOOST_TEST(w.size() == 3);
  BOOST_TEST(w(0) == 1);
  BOOST_TEST(w(1) == 1);
  BOOST_TEST(w(2) == 1);
}

BOOST_AUTO_TEST_CASE(static_coefs_test)
{
  auto v = Linx::vec<1, 2, 3>();
  BOOST_TEST(v.n == 3);
  BOOST_TEST(v.size() == 3);
  BOOST_TEST(v(0) == 1);
  BOOST_TEST(v(1) == 2);
  BOOST_TEST(v(2) == 3);

  using namespace Linx::Literals;

  auto w = Linx::vec<3_D, 1>();
  BOOST_TEST(w.n == 3);
  BOOST_TEST(w.size() == 3);
  BOOST_TEST(w(0) == 1);
  BOOST_TEST(w(1) == 1);
  BOOST_TEST(w(2) == 1);
}

BOOST_AUTO_TEST_CASE(zero_test)
{
  auto v = Linx::vec();
  BOOST_TEST(v.n == 0);
  BOOST_TEST(v.size() == 0);
  BOOST_TEST(v(8) == 0);

  using namespace Linx::Literals;

  auto w = Linx::vec<3_D>();
  BOOST_TEST(w.n == 3);
  BOOST_TEST(w.size() == 3);
  BOOST_TEST(w(0) == 0);
  BOOST_TEST(w(1) == 0);
  BOOST_TEST(w(2) == 0);
}

BOOST_AUTO_TEST_SUITE_END()
