// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE PixelwiseExample

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

#define ASSERT(...) BOOST_TEST((__VA_ARGS__))

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(intro_test)
{
  //! [intro]
  auto a = Linx::fill<4>("a", 3); // {3, 3, 3, 3}

  // Create new instances
  auto b = Linx::pow(a, 2); // {9, 9, 9, 9}
  auto c = Linx::pow(a, a); // {27, 27, 27, 27}
  auto d = Linx::pow(a, KOKKOS_LAMBDA(int i) { return i; }); // {1, 3, 9, 27}
  auto e = Linx::pow(a, Linx::Forward()); // {1, 3, 9, 27}

  ASSERT(a.contains_only(3));
  ASSERT(b.contains_only(9));
  ASSERT(c.contains_only(27));
  ASSERT(d == Linx::geometric<4>("expected", 1, 3));
  ASSERT(e == d);

  // Modify a in-place
  a.pow(a); // {27, 27, 27, 27}

  ASSERT(a.contains_only(27));
  //! [intro]
}

BOOST_AUTO_TEST_SUITE_END()
