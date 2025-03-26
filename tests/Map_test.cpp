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
  BOOST_CHECK_THROW(ckernel(-1, -1, -1), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()
