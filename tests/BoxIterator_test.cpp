// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE BoxCtorsTest

#include "Linx/Data/Box.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(box_iterator_test)
{
  using namespace Linx::Literals;

  auto region = Linx::cube<3_D, 2>();
  BOOST_CHECK(region.size() > 0);
  int size = 0;
  for (const auto& p : region) {
    BOOST_TEST(region.contains(p));
    ++size;
  }
  BOOST_TEST(size == region.size());
}

BOOST_AUTO_TEST_CASE(empty_box_iterator_test)
{
  using namespace Linx::Literals;

  auto region = Linx::shape<3_D, -1>();
  BOOST_CHECK(region.size() == 0);
  for (const auto& p : region) {
    std::cout << p << std::endl;
    throw std::out_of_range("We should not be there!");
  }
}

BOOST_AUTO_TEST_SUITE_END()
