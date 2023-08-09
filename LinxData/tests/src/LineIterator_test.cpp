// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Data/Line.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(LineIterator_test)

//-----------------------------------------------------------------------------

template <Index I, Index N>
void check_iterator(const Position<N>& front = Position<N>::one(), Index back = 12, Index step = 3) {
  const Line<I, N> slice(front, back, step);
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
  check_iterator<0, 1>();
  check_iterator<0, 4>();
  check_iterator<1, 4>();
  check_iterator<2, 4>();
  check_iterator<3, 4>();
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
