// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE BoxApplyTest

#include "Linx/Base/Functional.h"
#include "Linx/Base/Reduction.h"
#include "Linx/Data/Box.h"
#include "Linx/Run/ProgramContext.h"

#include <Kokkos_Core.hpp>
#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(count_test)
{
  using namespace Linx::Literals;

  const auto box = Linx::cube<4_D, 3>();
  BOOST_TEST(box.size() = 7 * 7 * 7 * 7);

  int count = 1;
  Linx::kokkos_reduce("count", box, Linx::Constant(1), Kokkos::Sum<int>(count));

  BOOST_TEST(count == box.size());
}

struct NegateFirstIndex {
  KOKKOS_INLINE_FUNCTION auto operator()(auto i0, auto...) const
  {
    return -i0;
  }
};

BOOST_AUTO_TEST_CASE(reduce_test)
{
  const auto box = Linx::shape<3, 4>();

  int sum = 1;
  kokkos_reduce("sum", box, NegateFirstIndex(), Kokkos::Sum<int>(sum));

  BOOST_TEST(sum == -12);
}

BOOST_AUTO_TEST_CASE(dynamic_rank_1_test)
{
  auto box = Linx::Box(Linx::vec({-1}), Linx::vec({1}));
  BOOST_TEST(box.n == -1);
  Linx::for_each<Kokkos::Serial>("test", box, [](int i) {
    BOOST_TEST(i >= -1);
    BOOST_TEST(i < 1);
  });
}

BOOST_AUTO_TEST_CASE(dynamic_rank_6_test)
{
  const auto box = Linx::Box(Linx::vec({-1, -2, -3, -4, -5, -6}), Linx::vec({1, 2, 3, 4, 5, 6}));
  BOOST_TEST(box.n == -1);
  Linx::for_each<Kokkos::Serial>("test", box, [](int i, int j, int k, int l, int m, int n) {
    BOOST_TEST(i >= -1);
    BOOST_TEST(i < 1);
    BOOST_TEST(j >= -2);
    BOOST_TEST(j < 2);
    BOOST_TEST(k >= -3);
    BOOST_TEST(k < 3);
    BOOST_TEST(l >= -4);
    BOOST_TEST(l < 4);
    BOOST_TEST(m >= -5);
    BOOST_TEST(m < 5);
    BOOST_TEST(n >= -6);
    BOOST_TEST(n < 6);
  });
}

BOOST_AUTO_TEST_CASE(static_rank_6_test)
{
  const auto box = Linx::Box({-1, -2, -3, -4, -5, -6}, {1, 2, 3, 4, 5, 6});
  BOOST_TEST(box.n == 6);
  BOOST_TEST(not box.static_flag);
  Linx::for_each<Kokkos::Serial>("test", box, [](int i, int j, int k, int l, int m, int n) {
    BOOST_TEST(i >= -1);
    BOOST_TEST(i < 1);
    BOOST_TEST(j >= -2);
    BOOST_TEST(j < 2);
    BOOST_TEST(k >= -3);
    BOOST_TEST(k < 3);
    BOOST_TEST(l >= -4);
    BOOST_TEST(l < 4);
    BOOST_TEST(m >= -5);
    BOOST_TEST(m < 5);
    BOOST_TEST(n >= -6);
    BOOST_TEST(n < 6);
  });
}

BOOST_AUTO_TEST_CASE(static_bounds_6_test)
{
  const auto box = Linx::Box(Linx::vec<-1, -2, -3, -4, -5, -6>(), Linx::vec<1, 2, 3, 4, 5, 6>());
  BOOST_TEST(box.n == 6);
  BOOST_TEST(box.static_flag);
  Linx::for_each<Kokkos::Serial>("test", box, [](int i, int j, int k, int l, int m, int n) {
    BOOST_TEST(i >= -1);
    BOOST_TEST(i < 1);
    BOOST_TEST(j >= -2);
    BOOST_TEST(j < 2);
    BOOST_TEST(k >= -3);
    BOOST_TEST(k < 3);
    BOOST_TEST(l >= -4);
    BOOST_TEST(l < 4);
    BOOST_TEST(m >= -5);
    BOOST_TEST(m < 5);
    BOOST_TEST(n >= -6);
    BOOST_TEST(n < 6);
  });
}

BOOST_AUTO_TEST_SUITE_END()
