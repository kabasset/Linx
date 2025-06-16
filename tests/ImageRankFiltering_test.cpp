// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ImageRankFilteringTest

#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/RankFiltering.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(min_test)
{
  const int width = 16;
  const int height = 9;
  const auto in = Linx::Image<int, 2>("input", width, height).fill_with_offsets_from_data();

  const int start = -1;
  const int stop = 2;
  const auto strel = Linx::Box<2>({start, start}, {stop, stop});
  const auto out = Linx::MinimumFilter(strel)(in);
  BOOST_TEST(out.extent(0) == width);
  BOOST_TEST(out.extent(1) == height);

  const auto& in_on_host = Linx::on_host(in);
  const auto& out_on_host = Linx::on_host(out);
  for (int j = -start; j <= height - stop; ++j) {
    for (int i = -start; i <= width - stop; ++i) {
      std::vector<int> neighbors;
      for (int l = start; l < stop; ++l) {
        for (int k = start; k < stop; ++k) {
          neighbors.push_back(in_on_host(i + k, j + l));
        }
      }
      BOOST_TEST(out_on_host(i, j) == *std::ranges::min_element(neighbors));
    }
  }
}

BOOST_AUTO_TEST_CASE(max_test)
{
  const int width = 16;
  const int height = 9;
  const auto in = Linx::Image<int, 2>("input", width, height).fill_with_offsets_from_data();

  const int start = -1;
  const int stop = 2;
  const auto strel = Linx::Box<2>({start, start}, {stop, stop});
  const auto out = Linx::MaximumFilter(strel)(in);
  BOOST_TEST(out.extent(0) == width);
  BOOST_TEST(out.extent(1) == height);

  const auto& in_on_host = Linx::on_host(in);
  const auto& out_on_host = Linx::on_host(out);
  for (int j = -start; j <= height - stop; ++j) {
    for (int i = -start; i <= width - stop; ++i) {
      std::vector<int> neighbors;
      for (int l = start; l < stop; ++l) {
        for (int k = start; k < stop; ++k) {
          neighbors.push_back(in_on_host(i + k, j + l));
        }
      }
      BOOST_TEST(out_on_host(i, j) == *std::ranges::max_element(neighbors));
    }
  }
}

BOOST_AUTO_TEST_CASE(median_test)
{
  const int width = 16;
  const int height = 9;
  const auto in = Linx::Image<int, 2>("input", width, height).fill_with_offsets_from_data();

  const int radius = 1;
  const auto strel = Linx::Box<2>({-radius, -radius}, {radius + 1, radius + 1});
  const auto out = Linx::MedianFilter(strel)(in);
  BOOST_TEST(out.extent(0) == width);
  BOOST_TEST(out.extent(1) == height);

  const auto& in_on_host = Linx::on_host(in);
  const auto& out_on_host = Linx::on_host(out);
  for (int j = radius; j < height - radius; ++j) {
    for (int i = radius; i < width - radius; ++i) {
      std::vector<int> neighbors;
      for (int l = -radius; l <= radius; ++l) {
        for (int k = -radius; k <= radius; ++k) {
          neighbors.push_back(in_on_host(i + k, j + l));
        }
      }
      BOOST_TEST(out_on_host(i, j) == Linx::median(neighbors));
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
