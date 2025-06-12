// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE BoxArithmeticTest

#include "Linx/Data/BoxUnion.h"
#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(position_position_additive_test)
{
  Linx::Position<2> in("in", {0, 1});
  Linx::Position<2> delta("delta", {-1, 1});

  auto plus = in + delta;
  BOOST_TEST((plus == Linx::Position<2> {-1, 2}));

  auto minus = in - delta;
  BOOST_TEST((minus == Linx::Position<2> {1, 0}));
}

BOOST_AUTO_TEST_CASE(scalar_additive_test)
{
  Linx::Box<2> in({0, 1}, {2, 3});

  auto plus = in + 1;
  BOOST_TEST((plus.start() == in.start() + 1));
  BOOST_TEST((plus.stop() == in.stop() + 1));

  auto minus = in - 1;
  BOOST_TEST((minus.start() == in.start() - 1));
  BOOST_TEST((minus.stop() == in.stop() - 1));
}

BOOST_AUTO_TEST_CASE(vector_additive_test)
{
  Linx::Box<2> in({0, 1}, {2, 3});
  Linx::Position<2> delta("delta", {-1, 1});

  auto plus = in + delta;
  BOOST_TEST((plus.start() == in.start() + delta));
  BOOST_TEST((plus.stop() == in.stop() + delta));

  auto minus = in - delta;
  BOOST_TEST((minus.start() == in.start() - delta));
  BOOST_TEST((minus.stop() == in.stop() - delta));
}

BOOST_AUTO_TEST_CASE(box_additive_test)
{
  auto in = Linx::Box<2>({-10, -1}, {2, 3});
  auto margin = Linx::Box<2>({-1, -2}, {2, 1});

  auto plus = in + margin;
  BOOST_TEST((plus.start() == in.start() + margin.start()));
  BOOST_TEST((plus.stop() == in.stop() + margin.stop()));
  BOOST_TEST((plus.shape() == in.shape() + margin.shape()));

  auto minus = in - margin;
  BOOST_TEST((minus.start() == in.start() - margin.start()));
  BOOST_TEST((minus.stop() == in.stop() - margin.stop()));
  BOOST_TEST((minus.shape() == in.shape() - margin.shape()));
}

BOOST_AUTO_TEST_CASE(box_difference_test)
{
  auto lhs = Linx::Box<2>({-2, 1}, {5, 4});
  auto rhs = Linx::Box<2>({0, 3}, {3, 8});
  auto vector = Linx::set_difference(lhs, rhs).boxes();
  BOOST_TEST(vector.size() == 3);
  auto expected = std::vector<Linx::Box<2>> {{{-2, 3}, {0, 4}}, {{3, 3}, {5, 4}}, {{-2, 1}, {5, 3}}};
  BOOST_TEST(vector == expected);
}

BOOST_AUTO_TEST_CASE(box_union_test)
{
  auto lhs = Linx::Box<2>({0, 3}, {3, 8});
  auto rhs = Linx::Box<2>({-2, 1}, {5, 4});
  auto vector = (lhs | rhs).boxes();
  BOOST_TEST(vector.size() == 4);
  auto expected = std::vector<Linx::Box<2>> {{{-2, 3}, {0, 4}}, {{3, 3}, {5, 4}}, {{-2, 1}, {5, 3}}, lhs};
  BOOST_TEST(vector == expected);
}

BOOST_AUTO_TEST_SUITE_END()
