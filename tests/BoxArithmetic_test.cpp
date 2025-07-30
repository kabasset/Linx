// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE BoxArithmeticTest

#include "Linx/Data/BoxRefactoring.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(scalar_additive_test)
{
  auto in = Linx::Box({0, 1}, {2, 3});

  auto plus = in + 1;
  BOOST_TEST((plus.start() == in.start() + 1));
  BOOST_TEST((plus.stop() == in.stop() + 1));

  auto minus = in - 1;
  BOOST_TEST((minus.start() == in.start() - 1));
  BOOST_TEST((minus.stop() == in.stop() - 1));
}

BOOST_AUTO_TEST_CASE(vector_additive_test)
{
  auto in = Linx::Box({0, 1}, {2, 3});
  auto delta = Linx::vec(-1, 1);

  auto plus = in + delta;
  BOOST_TEST((plus.start() == in.start() + delta));
  BOOST_TEST((plus.stop() == in.stop() + delta));

  auto minus = in - delta;
  BOOST_TEST((minus.start() == in.start() - delta));
  BOOST_TEST((minus.stop() == in.stop() - delta));
}

BOOST_AUTO_TEST_CASE(box_dilation_erosion_test)
{
  auto in = Linx::Box({0, 1}, {10, 5});
  auto margin = Linx::Box({-1}, {3});

  auto dilated = Linx::dilate(in, margin);
  BOOST_TEST(dilated.start().equal(-1, 1));
  BOOST_TEST(dilated.stop().equal(12, 5));

  auto eroded = Linx::erode(in, margin);
  BOOST_TEST(eroded.start().equal(1, 1));
  BOOST_TEST(eroded.stop().equal(8, 5));
}

BOOST_AUTO_TEST_CASE(intersection_test)
{
  using namespace Linx::Literals;

  auto lhs = Linx::shape<1, 3, 5, 7>() - Linx::vec<4_D, 2>(); // [-2, -2, -2, -2] ~ [-1, 1, 3, 5]
  auto rhs = Linx::cube<3_D, 1>(); // [-1, -1, -1] ~ [2, 2, 2]
  auto out = lhs & rhs;
  BOOST_TEST(out.static_size_flag);
  BOOST_TEST(out.start().equal(-1, -1, -1, -2));
  BOOST_TEST(out.stop().equal(-1, 1, 2, 5));
}

BOOST_AUTO_TEST_CASE(bbox_test)
{
  using namespace Linx::Literals;

  auto lhs = Linx::shape<1, 3, 5, 7>() - Linx::vec<4_D, 2>(); // [-2, -2, -2, -2] ~ [-1, 1, 3, 5]
  auto rhs = Linx::cube<3_D, 1>(); // [-1, -1, -1] ~ [2, 2, 2]
  auto out = Linx::bbox(lhs, rhs);
  BOOST_TEST(out.static_size_flag);
  BOOST_TEST(out.start().equal(-2, -2, -2, -2));
  BOOST_TEST(out.stop().equal(2, 2, 3, 5));
}

BOOST_AUTO_TEST_SUITE_END()
