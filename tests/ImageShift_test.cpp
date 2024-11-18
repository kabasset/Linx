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
  auto in = Linx::Image<int, 2>("in", 4, 3).fill_with_offsets();
  auto shift = Linx::Shift(in, 2, 1);
  BOOST_TEST((shift.offset() == Linx::Position<2>({2, 1}))); // FIXME <2> should be deduced
  BOOST_TEST((shift.domain() == in.domain() + shift.offset()));

  auto test = Linx::Image<int, 2>("test", in.shape());
  Linx::for_each(
      "test",
      in.domain(),
      KOKKOS_LAMBDA(int i, int j) { test(i, j) = shift(i - 2, j - 1) - in(i, j); });
  BOOST_TEST(Linx::norm<0>(test) == 0);
}

BOOST_AUTO_TEST_SUITE_END()
