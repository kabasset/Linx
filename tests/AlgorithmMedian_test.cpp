// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE AlgorithmMedian

#include "Linx/Base/Median.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(odd_increasing_test)
{
  const Linx::GPosition a({0, 1, 10, 100, 1000});
  BOOST_TEST(Linx::median<5>(a) == 10);
}

BOOST_AUTO_TEST_CASE(odd_decreasing_test)
{
  const Linx::GPosition a({1000, 100, 10, 1, 0});
  BOOST_TEST(Linx::median<5>(a) == 10);
}

BOOST_AUTO_TEST_CASE(odd_even_random_test)
{
  const Linx::GPosition a({1, 100, 0, 10, 10000, 1000});
  BOOST_TEST(Linx::median<5>(a) == 10); // Partial
  BOOST_TEST(Linx::median<6>(a) == 55);
}

BOOST_AUTO_TEST_SUITE_END()
