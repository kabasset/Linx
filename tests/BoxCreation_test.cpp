// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE BoxCtorsTest

#include "Linx/Data/BoxRefactoring.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(static_rank_test)
{
  using namespace Linx::Literals;

  auto stop = Linx::Box(Linx::vec(1, 2, 3));
  BOOST_TEST(stop.static_rank_flag);

  auto shape = Linx::shape(1, 2, 3);
  BOOST_TEST(shape.static_rank_flag);

  auto unit = Linx::shape<3_D>(1);
  BOOST_TEST(unit.static_rank_flag);

  auto box = Linx::Box(Linx::vec(-1, -2, -3), Linx::vec(2, 3, 4));
  BOOST_TEST(box.static_rank_flag);

  auto centered = Linx::cube(Linx::vec(1, 2, 3));
  BOOST_TEST(centered.static_rank_flag);

  auto cube = Linx::cube<3_D>(1);
  BOOST_TEST(cube.static_rank_flag);
}

BOOST_AUTO_TEST_CASE(static_size_test)
{
  using namespace Linx::Literals;

  auto stop = Linx::Box(Linx::vec<1, 2, 3>());
  BOOST_TEST(stop.static_size_flag);

  auto shape = Linx::shape<1, 2, 3>();
  BOOST_TEST(shape.static_size_flag);

  auto box = Linx::Box(Linx::vec<-1, -2, -3>(), Linx::vec<2, 3, 4>());
  BOOST_TEST(box.static_size_flag);

  auto centered = Linx::cube(Linx::vec<1, 2, 3>());
  BOOST_TEST(centered.static_size_flag);

  auto cube = Linx::cube<3_D, 1>();
  BOOST_TEST(cube.static_size_flag);

  // auto shifted = Linx::shape<1, 2, 3>() + Linx::vec<3_D, -1>();
}

BOOST_AUTO_TEST_SUITE_END()
