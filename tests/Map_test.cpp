// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE MapTest

#include "Linx/Data/Map.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>
#include <ranges>
#include <sstream>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(laplacian_kernel_test)
{
  static constexpr auto N = 3;
  using T = int;
  auto kernel = Linx::Map<T, N>("kernel");
  for (auto i : {0, 1, 2}) {
    auto p = Linx::Position<N>();
    kernel[p] += -2;
    p[i] = -1;
    kernel[p] = 1;
    p[i] = 1;
    kernel[p] = 1;
  }

  auto ckernel = as_readonly(kernel);
  BOOST_TEST(ckernel.ssize() == 2 * N + 1);
  BOOST_TEST(ckernel(-1, 0, 0) == 1);
  BOOST_TEST(ckernel(1, 0, 0) == 1);
  BOOST_TEST(ckernel(0, -1, 0) == 1);
  BOOST_TEST(ckernel(0, 1, 0) == 1);
  BOOST_TEST(ckernel(0, 0, -1) == 1);
  BOOST_TEST(ckernel(0, 0, 1) == 1);
  BOOST_TEST(ckernel(0, 0, 0) == -2 * N);
  BOOST_TEST(ckernel.at(-1, -1, -1) == ckernel.out_of_range);
}

BOOST_AUTO_TEST_CASE(map_iteration_test)
{
  static constexpr auto N = 2;
  using T = int;
  auto kernel = Linx::Map<T, N>();
  for (auto i : {0, 1, 2}) {
    auto p = Linx::Position<N>({i, 2 * i});
    kernel[p] = 1;
  }
  BOOST_TEST(kernel.size() == 3);
  BOOST_TEST(kernel.values().contains_only(1));
  for_each("inc", kernel.domain(), KOKKOS_LAMBDA(int i, int j) { kernel(i, j) += 1; });
  BOOST_TEST(kernel.values().contains_only(2));
}

BOOST_AUTO_TEST_SUITE_END()
