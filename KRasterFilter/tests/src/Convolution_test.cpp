// Copyright (C) 2022, Antoine Basset
// This file is part of Kast.Raster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "KRasterFilter/Convolution.h"

#include <boost/test/unit_test.hpp>

using namespace Kast;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Convolution_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(kernel_1d_init_test) {
  Kernel1d<int, OutOfBoundsCrop> kernel({0, 1, 2, 3, 4}, 1);
  BOOST_TEST(kernel.size() == 5);
  BOOST_TEST(kernel.backward() == 1);
  BOOST_TEST(kernel.forward() == 3);
  BOOST_TEST(*kernel.originData() == 1);
  int i = 0;
  for (auto it = kernel.begin(); it != kernel.end(); ++it, ++i) {
    BOOST_TEST(*it == i);
  }
}

template <Index I, typename TIn, typename TKernel, typename TOut>
void testConvolution1d(const TIn& in, const TKernel& kernel, const TOut& expected) {
  const auto out = kernel.template correlateAlong<I, typename TOut::Value>(in);
  BOOST_TEST(out.shape() == in.shape());
  for (const auto& p : out.domain()) {
    printf("%li,%li: %i\n", p[0], p[1], out[p]);
    BOOST_TEST(out[p] == expected[p]);
  }
}

BOOST_AUTO_TEST_CASE(convolution_1d_test) {
  const Position<2> shape {4, 3};
  VecRaster<int, 2> in(shape);
  Kernel1d<int, OutOfBoundsCrop> kernel({1, 2, 3}, 1);
  const int value = 10;
  std::fill(in.begin(), in.end(), value);
  const auto sum = std::accumulate(kernel.begin(), kernel.end(), 0);
  auto alongX = in * sum;
  auto alongY = in * sum;
  auto alongZ = in * sum;
  for (const auto& p : in.domain()) {
    const auto x = p[0];
    const auto y = p[1];
    const auto z = p[2];
    printf("%li,%li,%li:%i\n", x, y, z, in[p]);
    const auto k0 = value * kernel[0];
    const auto k2 = value * kernel[2];
    if (x == 0) {
      alongX[p] -= k0;
    }
    if (y == 0) {
      alongY[p] -= k0;
    }
    if (z == 0) {
      alongZ[p] -= k0;
    }
    if (x == shape[0] - 1) {
      alongX[p] -= k2;
    }
    if (y == shape[1] - 1) {
      alongY[p] -= k2;
    }
    if (z == shape[2] - 1) {
      alongZ[p] -= k2;
    }
  }
  testConvolution1d<0>(in, kernel, alongX);
  // testConvolution1d<1>(in, kernel, alongY);
  // testConvolution1d<2>(in, kernel, alongZ);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
