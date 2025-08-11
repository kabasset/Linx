// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE BoxUnionTest

#include "Linx/Data/Box.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(box_difference_test)
{
  auto lhs = Linx::Box({-2, 1}, {5, 4});
  auto rhs = Linx::Box({0, 3}, {3, 8});
  auto vector = Linx::set_difference(lhs, rhs).boxes();
  BOOST_TEST(vector.size() == 3);
  auto expected = std::vector {Linx::Box({-2, 3}, {0, 4}), Linx::Box({3, 3}, {5, 4}), Linx::Box({-2, 1}, {5, 3})};
  BOOST_TEST(vector == expected);
}

BOOST_AUTO_TEST_CASE(box_union_test)
{
  auto lhs = Linx::Box({0, 3}, {3, 8});
  auto rhs = Linx::Box({-2, 1}, {5, 4});
  auto vector = (lhs | rhs).boxes();
  BOOST_TEST(vector.size() == 4);
  auto expected = std::vector {Linx::Box({-2, 3}, {0, 4}), Linx::Box({3, 3}, {5, 4}), Linx::Box({-2, 1}, {5, 3}), lhs};
  BOOST_TEST(vector == expected);
}

BOOST_AUTO_TEST_SUITE_END()
