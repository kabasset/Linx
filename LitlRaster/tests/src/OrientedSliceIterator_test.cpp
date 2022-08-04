// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/OrientedSlice.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(OrientedSliceIterator_test)

//-----------------------------------------------------------------------------

template <Index I, Index N>
void checkIterator(const Position<N>& front = Position<N>::one(), Index back = 12, Index step = 3) {
  const OrientedSlice<I, N> slice(front, back, step);
  std::vector<Position<N>> expected(slice.size(), slice.front());
  for (std::size_t i = 0; i < expected.size(); ++i) {
    expected[i][I] += slice.step() * i;
  }
  std::vector<Position<N>> out;
  for (const auto& p : slice) {
    out.push_back(p);
  }
  BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(iterator_test) {
  checkIterator<0, 1>();
  checkIterator<0, 4>();
  checkIterator<1, 4>();
  checkIterator<2, 4>();
  checkIterator<3, 4>();
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
