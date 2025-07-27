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
  BOOST_TEST(stop.start().static_empty_flag);
  BOOST_TEST(stop.stop().equal(1, 2, 3));
  BOOST_TEST(stop.size() == 6);

  auto shape = Linx::shape(1, 2, 3);
  BOOST_TEST(shape.static_rank_flag);
  BOOST_TEST(shape.start().static_empty_flag);
  BOOST_TEST(shape.stop().equal(1, 2, 3));
  BOOST_TEST(shape.size() == 6);

  auto unit = Linx::shape<3_D>(1);
  BOOST_TEST(unit.static_rank_flag);
  BOOST_TEST(unit.start().static_empty_flag);
  BOOST_TEST(unit.stop().equal(1, 1, 1));
  BOOST_TEST(unit.size() == 1);

  auto box = Linx::Box(Linx::vec(-1, -2, -3), Linx::vec(2, 3, 4));
  BOOST_TEST(box.static_rank_flag);
  BOOST_TEST(box.start().equal(-1, -2, -3));
  BOOST_TEST(box.stop().equal(2, 3, 4));
  BOOST_TEST(box.size() == 105);

  auto centered = Linx::cube(Linx::vec(1, 2, 3));
  BOOST_TEST(centered.static_rank_flag);
  BOOST_TEST(centered.start().equal(-1, -2, -3));
  BOOST_TEST(centered.stop().equal(2, 3, 4));
  BOOST_TEST(centered.size() == 105);

  auto cube = Linx::cube<3_D>(1);
  BOOST_TEST(cube.static_rank_flag);
  BOOST_TEST(cube.start().equal(-1, -1, -1));
  BOOST_TEST(cube.stop().equal(2, 2, 2));
  BOOST_TEST(cube.size() == 27);
}

BOOST_AUTO_TEST_CASE(static_size_test)
{
  using namespace Linx::Literals;

  auto null = Linx::Box();
  BOOST_TEST(null.static_size_flag);
  BOOST_TEST(null.start().equal());
  BOOST_TEST(null.stop().equal());
  BOOST_TEST(null.size() == 1);

  auto empty = Linx::Box(Linx::vec<1_D>());
  BOOST_TEST(empty.static_size_flag);
  BOOST_TEST(empty.start().equal());
  BOOST_TEST(empty.stop().equal(0));
  BOOST_TEST(empty.size() == 0);

  auto stop = Linx::Box(Linx::vec<1, 2, 3>());
  BOOST_TEST(stop.static_size_flag);
  BOOST_TEST(stop.start().static_empty_flag);
  BOOST_TEST(stop.stop().equal(1, 2, 3));
  BOOST_TEST(stop.size() == 6);

  auto shape = Linx::shape<1, 2, 3>();
  BOOST_TEST(shape.static_size_flag);
  BOOST_TEST(shape.start().static_empty_flag);
  BOOST_TEST(shape.stop().equal(1, 2, 3));
  BOOST_TEST(shape.size() == 6);

  auto box = Linx::Box(Linx::vec<-1, -2, -3>(), Linx::vec<2, 3, 4>());
  BOOST_TEST(box.static_size_flag);
  BOOST_TEST(box.start().equal(-1, -2, -3));
  BOOST_TEST(box.stop().equal(2, 3, 4));
  BOOST_TEST(box.size() == 105);

  auto centered = Linx::cube(Linx::vec<1, 2, 3>());
  BOOST_TEST(centered.static_size_flag);
  BOOST_TEST(centered.start().equal(-1, -2, -3));
  BOOST_TEST(centered.stop().equal(2, 3, 4));
  BOOST_TEST(centered.size() == 105);

  auto cube = Linx::cube<3_D, 1>();
  BOOST_TEST(cube.static_size_flag);
  BOOST_TEST(cube.start().equal(-1, -1, -1));
  BOOST_TEST(cube.stop().equal(2, 2, 2));
  BOOST_TEST(cube.size() == 27);

  auto shifted = Linx::shape<1, 2, 3>() + Linx::vec<3_D, -1>();
  BOOST_TEST(shifted.static_size_flag);
  BOOST_TEST(shifted.start().equal(-1, -1, -1));
  BOOST_TEST(shifted.stop().equal(0, 1, 2));
}

BOOST_AUTO_TEST_SUITE_END()
