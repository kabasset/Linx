// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ImagePixelwiseTest

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(add_test)
{
  auto a = Linx::fill("a", 1, 4, 3);
  BOOST_TEST(a.contains_only(1));

  auto b = +a;
  BOOST_TEST(b.contains_only(1));
  ++b;
  BOOST_TEST(a.contains_only(1));
  BOOST_TEST(b.contains_only(2));

  auto c = a + b;
  BOOST_TEST(c.contains_only(3));
}

BOOST_AUTO_TEST_CASE(exp_test)
{
  auto a = Linx::fill("a", 0., 4, 3);
  BOOST_TEST(a.contains_only(0.));

  auto b = exp(a);
  BOOST_TEST(a.contains_only(0.));
  BOOST_TEST(b.contains_only(1.));

  a.exp();
  BOOST_TEST(a.contains_only(1.));
  BOOST_TEST(b.contains_only(1.));
}

BOOST_AUTO_TEST_SUITE_END()
