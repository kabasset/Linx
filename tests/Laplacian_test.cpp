// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE LaplacianTest

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/LinearFiltering.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(box_test)
{
  const auto filter = Linx::separable_laplacian<0, 1>();
  const auto box = Linx::box(filter.footprint());
  BOOST_TEST((box == Linx::Box<2>({-1, -1}, {2, 2})));
}

BOOST_AUTO_TEST_CASE(impulse_test)
{
  const auto in = Linx::Image<int, 3>("in", 5, 5, 5);
  in(2, 2, 2) = 1;
  const auto out = Linx::separable_laplacian<0, 1, 2>(-1)(in);
  const auto expected = Linx::Image<int, 3>("exp", 5, 5, 5);
  expected(2, 2, 1) = -1;
  expected(2, 1, 2) = -1;
  expected(1, 2, 2) = -1;
  expected(2, 2, 2) = 6;
  expected(3, 2, 2) = -1;
  expected(2, 3, 2) = -1;
  expected(2, 2, 3) = -1;
  BOOST_TEST((out == expected));
}

BOOST_AUTO_TEST_SUITE_END()
