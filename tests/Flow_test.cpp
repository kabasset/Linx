// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE FlowTest

#include "Linx/Base/Functional.h"
#include "Linx/Base/Random.h"
#include "Linx/Data/Image.h"
#include "Linx/Run/Flow.h"
#include "Linx/Run/Logging.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(sequence_api_test)
{
  auto size = 10;
  auto logger = Linx::TimerLogger();
  auto [out] =
      Linx::Flow("(1 + 2) * 3", logger)
          .append(Linx::Constant(1))
          .domain(Linx::Slice(0, size))
          .apply(Linx::Add(2), Linx::Multiply(3));
  logger("Done");
  BOOST_TEST(out.ssize() == size);
  BOOST_TEST(out.contains_only(9));
}

BOOST_AUTO_TEST_CASE(image_api_test)
{
  auto width = 10;
  auto height = 3;
  auto logger = Linx::TimerLogger();
  auto [out] =
      Linx::Flow("(1 + 2) * 3", logger)
          .append(Linx::Constant(1))
          .domain(Linx::Box({0, 0}, {width, height}))
          .apply(Linx::Add(2), Linx::Multiply(3));
  logger("Done");
  BOOST_TEST(out.shape().equal(width, height));
  BOOST_TEST(out.contains_only(9));
}

BOOST_AUTO_TEST_CASE(diadic_test)
{
  auto size = 100;
  auto logger = Linx::TimerLogger();
  auto [out] =
      Linx::Flow("1 + 2", logger)
          .append(Linx::Constant(1), Linx::Constant(2))
          .domain(Linx::Slice(0, size))
          .apply(Linx::Add());
  logger("Done");
  BOOST_TEST(out.contains_only(3));
}

BOOST_AUTO_TEST_CASE(append_test)
{
  auto [a, b, c, d] = Linx::Flow("Append").append('a', 'b').append('c', 'd');
  BOOST_TEST(a == 'a');
  BOOST_TEST(b == 'b');
  BOOST_TEST(c == 'c');
  BOOST_TEST(d == 'd');
}

BOOST_AUTO_TEST_CASE(prepend_test)
{
  auto [c, d, a, b] = Linx::Flow("Prepend").append('a', 'b').prepend('c', 'd');
  BOOST_TEST(c == 'c');
  BOOST_TEST(d == 'd');
  BOOST_TEST(a == 'a');
  BOOST_TEST(b == 'b');
}

BOOST_AUTO_TEST_SUITE_END()
