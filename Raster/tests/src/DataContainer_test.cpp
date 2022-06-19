// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Raster/DataContainer.h"
#include "Raster/Position.h"

#include <boost/test/unit_test.hpp>
#include <sstream>

using namespace Cnes;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(DataContainer_test)

//-----------------------------------------------------------------------------

template <long M, long N>
void checkStreamInsertion(const Position<N>& position, const std::string& expected) {
  const auto slice = position.template slice<M>();
  std::stringstream os;
  os << slice;
  BOOST_TEST(os.str() == expected);
}

BOOST_AUTO_TEST_CASE(stream_insertion_test) {
  Position<10> a {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  checkStreamInsertion<0>(a, "[]");
  checkStreamInsertion<1>(a, "[0]");
  checkStreamInsertion<2>(a, "[0, 1]");
  checkStreamInsertion<3>(a, "[0, 1, 2]");
  checkStreamInsertion<4>(a, "[0, 1, 2, 3]");
  checkStreamInsertion<5>(a, "[0, 1, 2, 3, 4]");
  checkStreamInsertion<6>(a, "[0, 1, 2, 3, 4, 5]");
  checkStreamInsertion<7>(a, "[0, 1, 2, 3, 4, 5, 6]");
  checkStreamInsertion<8>(a, "[0, 1, 2 ... 5, 6, 7]");
  checkStreamInsertion<9>(a, "[0, 1, 2 ... 6, 7, 8]");
}

BOOST_AUTO_TEST_CASE(quantiles_test) {

  Vector<double, 10> even;
  even.arange();
  BOOST_TEST(even.minmax().first == even.min());
  BOOST_TEST(even.minmax().second == even.max());
  BOOST_TEST(even.quantile(0) == even.min());
  BOOST_TEST(even.quantile(1) == even.max());
  BOOST_TEST(even.quantile(.5) == even.median());
  BOOST_TEST(even.median() == .5 * (even.nth(4) + even.nth(5)));

  Vector<double, 11> odd;
  odd.arange();
  BOOST_TEST(odd.minmax().first == odd.min());
  BOOST_TEST(odd.minmax().second == odd.max());
  BOOST_TEST(odd.quantile(0) == odd.min());
  BOOST_TEST(odd.quantile(1) == odd.max());
  BOOST_TEST(odd.quantile(.5) == odd.median());
  BOOST_TEST(odd.median() == odd.nth(5));
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
