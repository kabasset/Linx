// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ArraysExample

#include "Linx/Data/Image.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

#define ASSERT(...) BOOST_TEST((__VA_ARGS__))

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(sequence_size_test)
{
  //! [sequence_size]
  auto a = Linx::default_init<int, 3>("a");
  auto b = Linx::default_init<int>(3, "b");
  auto c = Linx::default_init<int>("c", 3, 4);

  ASSERT(a.rank() == 1);
  ASSERT(a.size() == 3);
  ASSERT(a.static_domain_flag);

  ASSERT(b.rank() == 1);
  ASSERT(b.size() == 3);

  ASSERT(c.rank() == 2);
  ASSERT(c.size() == 12);
  ASSERT(c.extent(0) == 3);
  ASSERT(c.extent(1) == 4);
  //! [sequence_size]
}

BOOST_AUTO_TEST_CASE(sequence_null_size_test)
{
  //! [sequence_null_size]
  // auto a = Linx::default_init<int, 0>("a"); // FIXME
  // ASSERT(a.rank() == 1);
  // ASSERT(a.size() == 0);
  // ASSERT(a.static_size_flag);

  auto b = Linx::default_init<int>("b", 0);
  ASSERT(b.rank() == 1);
  ASSERT(b.size() == 0);
  //! [sequence_null_size]
}

BOOST_AUTO_TEST_CASE(access_test)
{
  //! [access]
  auto a = Linx::Raster<int>("a", 4, 3);
  for (auto a_i : a) {
    ASSERT(a_i == 0);
  }

  a(0, 1) = 1;
  ASSERT(a(0, 1) == 1);

  auto p = Linx::vec(0, 1);
  a.at(p) = 2;
  ASSERT(a.at(p) == 2);
  //! [access]
}

BOOST_AUTO_TEST_SUITE_END()
