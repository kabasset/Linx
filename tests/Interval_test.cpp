// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE IntervalTest

#include "Linx/Base/Interval.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>
#include <sstream>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

template <bool Inf, bool Sup, typename T>
void test_unit_interval_size(T expected)
{
  auto interval = Linx::Slice<T, Linx::Between<Inf, Sup, T>>(0, 1);
  BOOST_TEST(interval.size() == expected);
}

BOOST_AUTO_TEST_CASE(unit_interval_size_test)
{
  test_unit_interval_size<true, true, int>(2);
  test_unit_interval_size<true, false, int>(1);
  test_unit_interval_size<false, true, int>(1);
  test_unit_interval_size<false, false, int>(0);
  test_unit_interval_size<true, true, float>(1);
  test_unit_interval_size<false, false, float>(1);
}

BOOST_AUTO_TEST_CASE(unbounded_test)
{
  auto interval = Linx::Slice();
  BOOST_TEST((kokkos_slice(interval) == Kokkos::ALL));

  BOOST_TEST(interval.contains(Linx::Limits<Linx::Index>::min()));
  BOOST_TEST(interval.contains(Linx::Limits<Linx::Index>::max()));

  auto str = (std::stringstream() << interval).str();
  BOOST_TEST(str == ":");
}

BOOST_AUTO_TEST_CASE(singleton_test)
{
  Linx::Index index = 10;
  auto interval = Linx::Slice(index);
  BOOST_TEST(interval.size() == 1);
  BOOST_TEST(interval.start() == index);
  BOOST_TEST(interval.stop() == index + 1);
  BOOST_TEST(kokkos_slice(interval) == index);

  BOOST_TEST(not interval.contains(index - Linx::Limits<Linx::Index>::epsilon()));
  BOOST_TEST(interval.contains(index));
  BOOST_TEST(not interval.contains(index + Linx::Limits<Linx::Index>::epsilon()));

  auto str = (std::stringstream() << interval).str();
  BOOST_TEST(str == std::to_string(index));
}

BOOST_AUTO_TEST_CASE(span_test)
{
  Linx::Index start = 3;
  Linx::Index stop = 14;
  Linx::Index size = stop - start;
  auto interval = Linx::Slice(start, stop);
  BOOST_TEST(interval.size() == size);
  BOOST_TEST(interval.start() == start);
  BOOST_TEST(interval.stop() == stop);
  BOOST_TEST(kokkos_slice(interval).first == start);
  BOOST_TEST(kokkos_slice(interval).second == stop);

  BOOST_TEST(not interval.contains(start - Linx::Limits<Linx::Index>::epsilon()));
  BOOST_TEST(interval.contains(start));
  BOOST_TEST(interval.contains(stop - Linx::Limits<Linx::Index>::epsilon()));
  BOOST_TEST(not interval.contains(stop));

  auto str = (std::stringstream() << interval).str();
  BOOST_TEST(str == std::to_string(start) + ':' + std::to_string(stop));
}

BOOST_AUTO_TEST_SUITE_END()
