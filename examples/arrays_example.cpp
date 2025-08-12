// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ArraysExample

#include "Linx/Data/Image.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

#define ASSERT(...) BOOST_TEST(__VA_ARGS__)

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(sequence_size_test)
{
  //! [sequence_size]
  auto a = Linx::Sequence<int, 3>();
  ASSERT(a.n == 1);
  ASSERT(a.size() == 3);
  ASSERT(a.rank() == 1);

  auto b = Linx::Sequence<int, -1>(3);
  ASSERT(b.n == 1);
  ASSERT(b.size() == 3);
  ASSERT(b.rank() == 1);
  //! [sequence_size]
}

BOOST_AUTO_TEST_CASE(sequence_null_size_test)
{
  //! [sequence_null_size]
  auto a = Linx::Sequence<int, 0>();
  ASSERT(a.n == 1);
  ASSERT(a.size() == 0);
  ASSERT(a.rank() == 1);

  auto b = Linx::Sequence<int, -1>(0);
  ASSERT(b.n == 1);
  ASSERT(b.size() == 0);
  ASSERT(b.rank() == 1);
  //! [sequence_null_size]
}

BOOST_AUTO_TEST_CASE(image_rank_test)
{
  //! [image_rank]
  auto a = Linx::default_init<int>("static", 10, 20, 30);
  ASSERT(a.n == 3);
  ASSERT(a.rank() == 3);
  ASSERT(a.size() == 10 * 20 * 30);
  ASSERT(a.extent(0) == 10);
  ASSERT(a.extent(1) == 20);
  ASSERT(a.extent(2) == 30);

  auto b = Linx::default_init<int>("dynamic", Linx::Box(Linx::vec({10, 20, 30}))); // FIXME ugly; useless?
  ASSERT(b.n == -1);
  ASSERT(b.rank() == 3);
  ASSERT(b.size() == 10 * 20 * 30);
  ASSERT(b.extent(0) == 10);
  ASSERT(b.extent(1) == 20);
  ASSERT(b.extent(2) == 30);
  //! [image_rank]
}

BOOST_AUTO_TEST_CASE(image_null_rank_test)
{
  //! [image_null_rank]
  auto a = Linx::default_init<int>("empty");
  ASSERT(a.n == 0);
  ASSERT(a.rank() == 0);
  ASSERT(a.size() == 1);
  ASSERT(not a.empty());
  ASSERT(a.data() == nullptr);
  //! [image_null_rank]
}

BOOST_AUTO_TEST_CASE(access_test)
{
  //! [access]
  auto a = Linx::Raster<int, 2>(4, 3);
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
