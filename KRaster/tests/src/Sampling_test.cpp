// Copyright (C) 2022, Antoine Basset
// This file is part of Kast.Raster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "KRaster/Sampling.h"

#include <boost/test/unit_test.hpp>
#include <numeric> // iota

using namespace Kast;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Sampling_test)

//-----------------------------------------------------------------------------

void testIndexSampling(Index from, Index to, Index by) {
  IndexSampling sampling {from, to, by};
  BOOST_TEST(sampling.size() == (to - from + by) / by);
  const auto vector = sampling.vector();
  BOOST_TEST(vector.size() == sampling.size());
  BOOST_TEST(vector.back() <= to);
  BOOST_TEST(vector.back() + by > to);
  std::size_t i = 0;
  Index current = from;
  for (auto index : sampling) {
    BOOST_TEST(index == vector[i]);
    BOOST_TEST(index == current);
    ++i;
    current += by;
  }
  BOOST_TEST(i == sampling.size());
}

BOOST_AUTO_TEST_CASE(index_sampling_test) {
  testIndexSampling(0, 1, 1);
  testIndexSampling(-1, 0, 1);
  testIndexSampling(0, 1, 2);
  testIndexSampling(1, 6, 2);
}

void testDataSamples(std::size_t size, Index from, Index to, Index by, Index stride) {
  std::vector<double> data(size);
  std::iota(data.begin(), data.end(), 0);
  DataSamples<double> samples(data.data(), data.size(), {from, to, by}, stride);
  BOOST_TEST(samples.size() == (to - from + by) / by);
  const auto vector = samples.vector();
  BOOST_TEST(vector.size() == samples.size());
  std::size_t i = 0;
  std::size_t current = from * stride;
  auto* it = &data[current];
  for (auto value : samples) {
    BOOST_TEST(value == vector[i]);
    BOOST_TEST(value == current);
    BOOST_TEST(value == *it);
    ++i;
    current += by * stride;
    it += by * stride;
  }
  BOOST_TEST(i == samples.size());
}

BOOST_AUTO_TEST_CASE(data_sample_test) {
  testDataSamples(10, 0, 1, 1, 1);
  testDataSamples(10, 0, 1, 2, 1);
  testDataSamples(10, 1, 6, 2, 1);
  testDataSamples(30, 0, 1, 1, 3);
  testDataSamples(30, 0, 1, 2, 3);
  testDataSamples(30, 1, 6, 2, 3);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
