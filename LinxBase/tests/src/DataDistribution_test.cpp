// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LinxBase/DataContainer.h"

#include <boost/test/unit_test.hpp>
#include <sstream>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(DataDistribution_test)

//-----------------------------------------------------------------------------

template <Index Size>
void checkQuantiles() {
  MinimalDataContainer<double> data(Size);
  data.range();
  auto dist = data.distribution();
  BOOST_TEST(dist.min() == 0);
  BOOST_TEST(dist.max() == Size - 1);
  BOOST_TEST(dist.quantile(0) == dist.min());
  BOOST_TEST(dist.quantile(1) == dist.max());
  BOOST_TEST(dist.quantile(.5) == dist.median());
  if (Size % 2 == 0) {
    BOOST_TEST(dist.median() == .5 * (dist.nth(Size / 2 - 1) + dist.nth(Size / 2)));
  } else {
    BOOST_TEST(dist.median() == dist.nth(Size / 2));
  }
}

BOOST_AUTO_TEST_CASE(quantiles_test) {
  checkQuantiles<10>();
  checkQuantiles<13>();
}

BOOST_AUTO_TEST_CASE(robust_test) {
  MinimalDataContainer<int> data {2, 1, 9, 4, 1, 2, 6};
  auto dist = data.distribution();
  BOOST_TEST(dist.min() == 1);
  BOOST_TEST(dist.max() == 9);
  BOOST_TEST(dist.median() == 2);
  BOOST_TEST(dist.mad() == 1);
}

BOOST_AUTO_TEST_CASE(histogram_test) {
  MinimalDataContainer<int> data(10);
  data.range();
  const std::vector<double> bins {-10, -.5, 0, 1.5, 4, 9, 12};
  const auto hist = data.distribution().histogram(bins);
  const std::vector<std::size_t> expected {0, 0, 2, 2, 5, 1};
  BOOST_TEST(hist == expected);
  // FIXME better test
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
