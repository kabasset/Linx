// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE AlgorithmMedian

#include "Linx/Base/Median.h"
#include "Linx/Data/Vector.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(fixed_odd_increasing_test)
{
  auto a = Linx::vec(0, 1, 10, 100, 1000);
  BOOST_TEST(Linx::median(a) == 10);
}

BOOST_AUTO_TEST_CASE(dynamic_odd_increasing_test)
{
  auto a = Linx::vec({0, 1, 10, 100, 1000});
  BOOST_TEST(Linx::median(a) == 10);
}

BOOST_AUTO_TEST_CASE(fixed_odd_decreasing_test)
{
  auto a = Linx::vec(1000, 100, 10, 1, 0);
  BOOST_TEST(Linx::median(a) == 10);
}

BOOST_AUTO_TEST_CASE(fixed_even_random_test)
{
  auto a = Linx::vec(1, 100, 0, 10, 10000, 1000);
  BOOST_TEST(Linx::median(a) == 55);
}

BOOST_AUTO_TEST_CASE(dynamic_even_random_test)
{
  auto a = Linx::vec({1, 100, 0, 10, 10000, 1000});
  BOOST_TEST(Linx::median(a) == 55);
}

BOOST_AUTO_TEST_CASE(partial_odd_random_test)
{
  auto a = Linx::vec(1, 100, 0, 10, 10000, 1000);
  BOOST_TEST(Linx::median<5>(a) == 10);
}

BOOST_AUTO_TEST_CASE(no_selectnet_test)
{
  using namespace Linx::Literals;

  auto a = Linx::vec<100_D>(0.);
  for (int i = 0; i < a.size(); ++i) {
    a[i] = i;
  }
  BOOST_TEST(Linx::median(a) == 49.5);
}

BOOST_AUTO_TEST_SUITE_END()
