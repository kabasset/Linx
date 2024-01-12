// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Transforms/Filters.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Filters_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(full_test)
{
  // FIXME make fixture
  auto in = Raster<int>({4, 3}).fill(1);
  auto k = convolution(Raster<int>({3, 3}).fill(1));
  auto out = k * extrapolation(in, 0);
  std::vector<int> expected = {4, 6, 6, 4, 6, 9, 9, 6, 4, 6, 6, 4};
  for (std::size_t i = 0; i < 12; ++i) {
    BOOST_TEST(out[i] == expected[i]);
  }
}

BOOST_AUTO_TEST_CASE(crop_test)
{
  const auto in = Raster<int>({10, 8}).range();
  const auto extrapolated = extrapolation(in, 0);
  const auto k = convolution(Raster<int>({3, 3}).fill(1));
  const auto expected = k * extrapolated;

  const auto out = k * in;
  const auto region = in.domain() - k.window();
  BOOST_TEST(out.shape() == region.shape());
  for (const auto& p : out.domain()) {
    BOOST_TEST(out[p] == expected[p + region.front()]);
  }
}

BOOST_AUTO_TEST_CASE(inner_box_test)
{
  const auto in = Raster<int, 3>({5, 6, 7}).range();
  const auto extrapolated = extrapolation(in, 0);
  const auto k = convolution(Raster<int, 3>({3, 3, 3}).fill(1));
  const auto expected = k * extrapolated;

  const Box<3> region {Position<3>::one(), in.shape() - 2}; // No extrapolation needed
  BOOST_TEST(region <= (in.domain() - k.window())); // inner
  const auto out = k * in(region);
  BOOST_TEST(out.shape() == region.shape());
  for (const auto& p : out.domain()) {
    BOOST_TEST(out[p] == expected[p + region.front()]);
  }
}

// FIXME extrapolated_box_test, inc. out-of-domain positions

BOOST_AUTO_TEST_CASE(inner_decimate_test)
{
  const auto in = Raster<int, 3>({5, 6, 7}).range();
  const auto extrapolated = extrapolation(in, 0);
  const auto k = convolution(Raster<int, 3>({3, 3, 3}).fill(1));
  const auto expected = k * extrapolated;

  const Box<3> crop {Position<3>::one(), in.shape() - 2};
  const auto region = Grid<3>(crop, Position<3>::one() * 3);
  const auto out = k * in(region);
  BOOST_TEST(out.shape() == region.shape());
  for (const auto& p : out.domain()) {
    BOOST_TEST(out[p] == expected[region.front() + p * 3]);
  }
}

BOOST_AUTO_TEST_CASE(extrapolated_decimate_1d_test)
{
  const auto in = Raster<int, 1>({13}).range();
  const auto extrapolated = extrapolation(in, 0);
  const auto k = convolution(Raster<int, 1>({7}).fill(1));
  const auto expected = k * extrapolated;

  const auto region = Grid<1>({Position<1> {1}, Position<1> {10}}, Position<1> {3});
  const auto out = k * extrapolated(region);
  BOOST_TEST(out.shape() == region.shape());
  for (std::size_t i = 0; i < out.size(); ++i) {
    BOOST_TEST(out[i] == expected[1 + i * 3]);
  }
}

BOOST_AUTO_TEST_CASE(extrapolated_decimate_2d_test)
{
  const auto in = Raster<int, 2>({13, 13}).range();
  const auto extrapolated = extrapolation(in, 0);
  const auto k = convolution(Raster<int, 2>({7, 7}).fill(1));
  const auto expected = k * extrapolated;

  const auto region = Grid<2>({Position<2> {1, 1}, Position<2> {10, 10}}, Position<2> {3, 3});
  const auto out = k * extrapolated(region);
  BOOST_TEST(out.shape() == region.shape());
  for (const auto& p : out.domain()) {
    BOOST_TEST(out[p] == expected[region.front() + p * 3]);
  }
}

BOOST_AUTO_TEST_CASE(extrapolated_decimate_3d_test)
{
  const auto in = Raster<int, 3>({5, 6, 7}).range();
  const auto extrapolated = extrapolation(in, 0);
  const auto k = convolution(Raster<int, 3>({3, 3, 3}).fill(1));
  const auto expected = k * extrapolated;

  const auto region = Grid<3>(in.domain(), Position<3>::one() * 3);
  const auto out = k * extrapolated(region);
  BOOST_TEST(out.shape() == region.shape());
  for (const auto& p : out.domain()) {
    BOOST_TEST(out[p] == expected[region.front() + p * 3]);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
