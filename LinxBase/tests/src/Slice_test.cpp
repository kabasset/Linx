// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LinxBase/Slice.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Slice_test)

//-----------------------------------------------------------------------------

void checkSlice(const Slice& in, Index front, Index back, Index step, std::size_t size) {
  BOOST_TEST(in.front() == front);
  BOOST_TEST(in.back() == back);
  BOOST_TEST(in.step() == step);
  BOOST_TEST(in.size() == size);
}

BOOST_AUTO_TEST_CASE(segment_test) {
  // 2, 3, 4, ... , 10
  checkSlice({2, 10}, 2, 10, 1, 9);
}

BOOST_AUTO_TEST_CASE(segment_from_size_test) {
  checkSlice(Slice::fromSize(2, 9), 2, 10, 1, 9);
}

BOOST_AUTO_TEST_CASE(slice_test) {
  // 2, 6, 10
  checkSlice({2, 10, 4}, 2, 10, 4, 3);
  checkSlice({2, 11, 4}, 2, 10, 4, 3);
  checkSlice({2, 12, 4}, 2, 10, 4, 3);
  checkSlice({2, 13, 4}, 2, 10, 4, 3);
}

BOOST_AUTO_TEST_CASE(slice_from_shape_test) {
  checkSlice(Slice::fromSize(2, 3, 4), 2, 10, 4, 3);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
