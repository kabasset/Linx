// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlTransforms/Kernel.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Kernel_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(coverage_test) {
  auto in = Raster<int>({4, 3}).fill(1);
  auto k = kernelize(Raster<int>({3, 3}).fill(1));
  auto out = k * extrapolate(in, 0);
  std::vector<int> expected = {4, 6, 6, 4, 6, 9, 9, 6, 4, 6, 6, 4};
  for (std::size_t i = 0; i < 12; ++i) {
    BOOST_TEST(out[i] == expected[i]);
  }
}

BOOST_AUTO_TEST_CASE(inner_box_test) {

  const auto in = Raster<int>({10, 8}).range();
  const auto extrapolated = extrapolate(in, 0);
  const auto k = kernelize(Raster<int>({3, 3}).fill(1));
  const auto expected = k * extrapolated;

  const auto region = Box<2>::fromCenter(2, in.shape() / 2);
  const auto regionOut = k * extrapolated.patch(region);
  BOOST_TEST(regionOut.shape() == in.shape());
  for (const auto& p : regionOut.domain()) {
    if (region[p]) {
      BOOST_TEST(regionOut[p] == expected[p]);
    } else {
      BOOST_TEST(regionOut[p] == 0);
    }
  }
}

BOOST_AUTO_TEST_CASE(regionwise_test) {

  // FIXME make fixture
  const auto in = Raster<int, 3>({5, 6, 7}).range();
  const auto extrapolated = extrapolate(in, 0);
  const auto k = kernelize(Raster<int, 3>({3, 3, 3}).fill(1));
  const auto expected = k * extrapolated;

  const auto region = Mask<3>::ball<2>(2, in.shape() / 2);
  BOOST_TEST(region.box() <= in.domain());
  const auto regionOut = k * extrapolated.patch(region);
  BOOST_TEST(regionOut.shape() == in.shape());
  for (const auto& p : regionOut.domain()) {
    if (region[p]) {
      BOOST_TEST(regionOut[p] == expected[p]);
    } else {
      BOOST_TEST(regionOut[p] == 0);
    }
  }
}

BOOST_AUTO_TEST_CASE(crop_test) {

  const auto in = Raster<int, 3>({5, 6, 7}).range();
  const auto extrapolated = extrapolate(in, 0);
  const auto k = kernelize(Raster<int, 3>({3, 3, 3}).fill(1));
  const auto expected = k * extrapolated;

  std::cout << "CROP\n";
  const Box<3> crop {Position<3>::one(), in.shape() - 2}; // No extrapolation needed
  const auto cropOut = k.correlateCrop(in.patch(crop));
  BOOST_TEST(cropOut.shape() == crop.shape());
  for (const auto& p : cropOut.domain()) {
    BOOST_TEST(cropOut[p] == expected[p + crop.front()]);
  }
}

BOOST_AUTO_TEST_CASE(decimate_test) {

  const auto in = Raster<int, 3>({5, 6, 7}).range();
  const auto extrapolated = extrapolate(in, 0);
  const auto k = kernelize(Raster<int, 3>({3, 3, 3}).fill(1));
  const auto expected = k * extrapolated;

  std::cout << "DECIMATE\n";
  const Box<3> crop {Position<3>::one(), in.shape() - 2}; // No extrapolation needed
  const auto decimate = Grid<3>(crop, Position<3>::one() * 3);
  const auto decimateOut = k.correlateDecimate(in.patch(decimate));
  BOOST_TEST(decimateOut.shape() == decimate.shape());
  for (const auto& p : decimateOut) {
    BOOST_TEST(decimateOut[p] == expected[decimate.front() + p * 3]);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
