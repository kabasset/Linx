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
  const int radius = 3 * sigma;
  auto k = Linx::sampled_gaussian_kernel(sigma, radius);
  BOOST_TEST(k.size() == 2 * radius + 1);
  BOOST_TEST(k.domain().start(0) == -radius);
  BOOST_TEST(k.domain().stop(0) == radius + 1);

  const auto& k_h = Linx::on_host(k);
  for (auto i = 0; i < radius; ++i) {
    BOOST_TEST(k_h(i) > k_h(i + 1));
    BOOST_TEST(k_h(i) == k_h(-i));
  }
  BOOST_TEST(Linx::sum(k) <= 1);
  BOOST_TEST(Linx::sum(k) >= 0.999);
}

BOOST_AUTO_TEST_CASE(raster_impulse_test)
{
  const float sigma = 1;
  const int radius = 5;
  auto k = Linx::on_host(Linx::sampled_gaussian_kernel(sigma, radius));
  auto conv = Linx::Convolution(k);
  auto domain = Linx::shape(2 * radius + 1, 1);
  auto in = Linx::Raster<float, decltype(domain)>("in", domain);
  in(radius, 0) = 1;
  auto out = conv(in);
  BOOST_TEST(out.size() == in.size());
  for (int i = 0; i < in.ssize(); ++i) {
    if (i == radius) {
      BOOST_TEST(in(i, 0) == 1);
      BOOST_TEST(out(i, 0) == k(0));
    } else {
      BOOST_TEST(out(i, 0) == 0);
    }
  }
}

BOOST_AUTO_TEST_CASE(extrapolated_impulse_test)
{
  using namespace Linx::Literals;

  const float sigma = 3;
  const int radius = 10 * sigma;
  auto k = Linx::sampled_gaussian_kernel(sigma, radius);
  auto conv = Linx::Convolution(k).pad(0);
  auto domain = Linx::cube<1_D, 0>();
  auto in = Linx::fill("in", 1, domain);
  auto out = conv(in);
  const auto& k_h = Linx::on_host(k);
  const auto& out_h = Linx::on_host(out);
  BOOST_TEST(out_h(0) == k_h(0));
}

BOOST_AUTO_TEST_SUITE_END()
