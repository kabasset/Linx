// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE GaussianFilterTest

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/LinearFiltering.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(kernel_test)
{
  const float sigma = 1;
  const int radius = 10 * sigma;
  auto k = Linx::sampled_gaussian_kernel(sigma, radius);
  BOOST_TEST(k.size() == 2 * radius + 1);
  BOOST_TEST(k.domain().start(0) == -radius);
  BOOST_TEST(k.domain().stop(0) == radius + 1);

  const auto& k_h = Linx::on_host(k);
  for (auto i = 0; i < radius; ++i) {
    BOOST_TEST(k_h(i) > k_h(i + 1));
    BOOST_TEST(k_h(i) == k_h(-i));
  }
}

BOOST_AUTO_TEST_SUITE_END()
