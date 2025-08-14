// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ProfileTest

#include "Linx/Data/Image.h"
#include "Linx/Data/Profile.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE) // FIXME to some Testing.h

BOOST_AUTO_TEST_CASE(iteration_test)
{
  auto a = Linx::arithmetic<8, Kokkos::HostSpace>("a", 0, 1);
  auto profile = Linx::Profile(a, 10);
  for (int i = 0; i < 5; ++i) {
    profile.push_back(i);
  }

  BOOST_TEST(profile.size() == 5);

  for (int i = 0; i < 5; ++i) {
    BOOST_TEST(profile[i] == i);
  }

  auto it = profile.begin();
  for (int i = 0; i < 5; ++i) {
    BOOST_TEST(it[i] == i);
  }

  int expected = 0;
  for (const auto& e : profile) {
    BOOST_TEST(e == expected);
    ++expected;
  }
}

BOOST_AUTO_TEST_CASE(filter_test)
{
  auto a = Linx::generate("a", KOKKOS_LAMBDA(int i, int j) { return i + 4 * j; }, 4, 3);
  auto evens = Linx::filter(a, KOKKOS_LAMBDA(int a_i) { return a_i % 2 == 0; });
  BOOST_TEST(evens.size() == (a.size() + 1) / 2);
  BOOST_TEST(Linx::sum(evens) == evens.size() * (evens.size() - 1));
}

BOOST_AUTO_TEST_SUITE_END()
