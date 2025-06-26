// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE SequenceCreationTest

#include "Linx/Data/Sequence.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

template <typename T>
void check_arithmetic(auto&&... args)
{
  auto ssized = Linx::arithmetic<3>("ssized", LINX_FORWARD(args)...);
  BOOST_TEST(on_host(ssized).equal(1, 2, 3)); // FIXME on device
  static_assert(std::is_same_v<typename decltype(ssized)::value_type, T>);
  auto dsized = Linx::arithmetic(3, "dsized", LINX_FORWARD(args)...);
  BOOST_TEST(on_host(dsized).equal(1, 2, 3)); // FIXME on device
  static_assert(std::is_same_v<typename decltype(dsized)::value_type, T>);
}

template <typename T>
void check_geometric(auto&&... args)
{
  auto ssized = Linx::geometric<3>("ssized", LINX_FORWARD(args)...);
  BOOST_TEST(on_host(ssized).equal(2, 4, 8)); // FIXME on device
  static_assert(std::is_same_v<typename decltype(ssized)::value_type, T>);
  auto dsized = Linx::geometric(3, "dsized", LINX_FORWARD(args)...);
  BOOST_TEST(on_host(dsized).equal(2, 4, 8)); // FIXME on device
  static_assert(std::is_same_v<typename decltype(dsized)::value_type, T>);
}

BOOST_AUTO_TEST_CASE(arithmetic_test)
{
  check_arithmetic<int>(Linx::Slice(1, 4));
  check_arithmetic<double>(Linx::Slice(1., 4));
  check_arithmetic<int>(Linx::Segment<int>(1, 3));
  check_arithmetic<double>(Linx::Segment<double>(1, 3));
  check_arithmetic<int>(1, Linx::Add(1));
  check_arithmetic<double>(1, Linx::Subtract(-1.));
}

BOOST_AUTO_TEST_CASE(geometric_test)
{
  // check_geometric<int>(Linx::Slice(1, 4));
  // check_geometric<double>(Linx::Slice(1., 4));
  // check_geometric<int>(Linx::Segment<int>(1, 3));
  // check_geometric<double>(Linx::Segment<double>(1, 3));
  // FIXME implement interval-based geometric()
  check_geometric<int>(2, Linx::Multiply(2));
  check_geometric<double>(2, Linx::Divide(0.5));
}

BOOST_AUTO_TEST_SUITE_END()
