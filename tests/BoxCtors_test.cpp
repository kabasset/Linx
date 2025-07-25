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

  Linx::Box(Linx::vec(1, 2, 3));
  Linx::Box({1, 2, 3});
  Linx::shape(1, 2, 3);
  Linx::shape<1, 2, 3>();
  Linx::shape<3_D>(1);
  // Linx::shape<1, 2, 3>() + Linx::vec<3_D>(1);
  Linx::Box(Linx::vec(-1, -2, -3), Linx::vec(2, 3, 4));
  Linx::Box({-1, -2, -3}, {2, 3, 4});
  Linx::cube(Linx::vec(1, 2, 3));
  Linx::cube(Linx::vec<1, 2, 3>());
}

BOOST_AUTO_TEST_SUITE_END()
