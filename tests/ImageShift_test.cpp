// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ImageShiftTest

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/Shift.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(positive_offset_test)
{
  auto in = Linx::Image<int, 2>("in", 4, 3).fill_with_distance_from_data();
  auto shift = Linx::Shift(in, 2, 1);
  BOOST_TEST((shift.offset() == Linx::Position<2>({2, 1}))); // FIXME <2> should be deduced
  BOOST_TEST((shift.domain() == in.domain() + shift.offset()));

  auto test = Linx::Image<int, 2>("test", in.shape());
  Linx::for_each(
      "test",
      shift.domain(),
      KOKKOS_LAMBDA(int i, int j) { test(i - 2, j - 1) = shift(i, j) - in(i - 2, j - 1); });
  BOOST_TEST(Linx::norm<0>(test) == 0);
}

BOOST_AUTO_TEST_CASE(negative_offset_distance_test)
{
  auto in = Linx::Position<4>("in", 4).fill_with_distance_from_data();
  auto shift = Linx::Shift(in, -2); // FIXME domain should be a Slice
  BOOST_TEST(shift.distance_from_origin(0) == 0);
  const auto offset = &shift(0) - &in(0);
  BOOST_TEST(offset == 2);
  for (int i = shift.domain().start()[0]; i < shift.domain().stop()[0]; ++i) {
    BOOST_TEST(shift.distance_from_origin(i) == i);
  };
}

BOOST_AUTO_TEST_SUITE_END()
