// Copyright (C) 2022, Antoine Basset
// This file is part of Kast.Raster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "KRaster/Sampling.h"

#include <boost/test/unit_test.hpp>

using namespace Kast;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Sampling_test)

//-----------------------------------------------------------------------------

void testIndexSampling(Index from, Index to, Index by) {
  IndexSampling sampling {from, to, by};
  BOOST_TEST(sampling.size() == (to - from + by) / by);
  const auto indices = sampling.vector();
  BOOST_TEST(indices.size() == sampling.size());
  BOOST_TEST(indices.back() <= to);
  BOOST_TEST(indices.back() + by > to);
  std::size_t i = 0;
  for (auto index : sampling) {
    BOOST_TEST(index == indices[i]);
    ++i;
  }
  BOOST_TEST(i == sampling.size());
}

BOOST_AUTO_TEST_CASE(index_sampling_test) {
  testIndexSampling(0, 1, 1);
  testIndexSampling(-1, 0, 1);
  testIndexSampling(0, 1, 2);
  testIndexSampling(0, 5, 2);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
