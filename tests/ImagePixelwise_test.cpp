// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ImagePixelwiseTest

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(offset_test) // FIXME somewhere else?
{
  auto in = Linx::Image<bool, 8>(Linx::Position({1, 2, 3, 4, 5, 6, 7, 8}));
  auto strides = in.strides();
  BOOST_TEST(in.offset(8, 0, 0, 0, 0, 0, 0, 0) == 8 * strides[0]);
  BOOST_TEST(in.offset(0, 7, 0, 0, 0, 0, 0, 0) == 7 * strides[1]);
  BOOST_TEST(in.offset(0, 0, 6, 0, 0, 0, 0, 0) == 6 * strides[2]);
  BOOST_TEST(in.offset(0, 0, 0, 5, 0, 0, 0, 0) == 5 * strides[3]);
  BOOST_TEST(in.offset(0, 0, 0, 0, 4, 0, 0, 0) == 4 * strides[4]);
  BOOST_TEST(in.offset(0, 0, 0, 0, 0, 3, 0, 0) == 3 * strides[5]);
  BOOST_TEST(in.offset(0, 0, 0, 0, 0, 0, 2, 0) == 2 * strides[6]);
  BOOST_TEST(in.offset(0, 0, 0, 0, 0, 0, 0, 1) == 1 * strides[7]);
  BOOST_TEST(in.offset(-1, -1, -1, -1, -1, -1, -1, -1) == -Linx::sum(strides));

  const auto& in_on_host = Linx::on_host(in);
  auto front = &in_on_host.front();
  BOOST_TEST(&in_on_host(1, 0, 0, 0, 0, 0, 0, 0) - front == strides[0]);
  BOOST_TEST(&in_on_host(0, 1, 0, 0, 0, 0, 0, 0) - front == strides[1]);
  BOOST_TEST(&in_on_host(0, 0, 1, 0, 0, 0, 0, 0) - front == strides[2]);
  BOOST_TEST(&in_on_host(0, 0, 0, 1, 0, 0, 0, 0) - front == strides[3]);
  BOOST_TEST(&in_on_host(0, 0, 0, 0, 1, 0, 0, 0) - front == strides[4]);
  BOOST_TEST(&in_on_host(0, 0, 0, 0, 0, 1, 0, 0) - front == strides[5]);
  BOOST_TEST(&in_on_host(0, 0, 0, 0, 0, 0, 1, 0) - front == strides[6]);
  BOOST_TEST(&in_on_host(0, 0, 0, 0, 0, 0, 0, 1) - front == strides[7]);
}

BOOST_AUTO_TEST_CASE(sum_test)
{
  const int width = 4;
  const int height = 3;
  using Image = Linx::Image<int, 2>;

  Image a("a", width, height);
  a.fill(1);
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
  const int width = 4;
  const int height = 3;
  using Image = Linx::Image<double, 2>;

  Image a("a", width, height);
  BOOST_TEST(a.contains_only(0.));

  auto b = exp(a); // FIXME add name as first argument
  BOOST_TEST(a.contains_only(0.));
  BOOST_TEST(b.contains_only(1.));

  a.exp();
  BOOST_TEST(a.contains_only(1.));
  BOOST_TEST(b.contains_only(1.));
}

BOOST_AUTO_TEST_SUITE_END()
