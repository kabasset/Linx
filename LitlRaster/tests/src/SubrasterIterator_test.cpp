// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Box.h"
#include "LitlRaster/Grid.h"
#include "LitlRaster/Mask.h"
#include "LitlRaster/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(SubrasterIterator_test)

//-----------------------------------------------------------------------------

template <Index N, typename TRegion>
void checkIterator(const Position<N>& shape, const TRegion& region) {

  const auto raster = Raster<float, N>(shape).range();

  std::vector<float> expected;
  expected.reserve(region.size());
  for (const auto& p : region) {
    expected.push_back(raster[p]);
  }

  std::vector<float> out;
  out.reserve(expected.size());
  for (const auto& v : raster.subraster(region)) {
    out.push_back(v);
  }

  BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(inner_cube_iterator_test) {
  const Position<3> shape {4, 5, 6};
  const Box<3> box {{1, 1, 1}, {2, 3, 4}};
  checkIterator(shape, box);
  checkIterator(shape, Grid<3>(box, Position<3>::one()));
  checkIterator(shape, Mask<3>(box, true));
  checkIterator(shape, Sequence<Position<3>>(box));
}

template <Index I, Index N>
void checkSliceIterator(const Position<N>& shape) {
  OrientedSlice<I, 3> slice(Position<N>::one(), shape[I] - 1, 2);
  checkIterator(shape, slice);
}

BOOST_AUTO_TEST_CASE(slice_iterator_test) {
  const Position<3> shape {4, 5, 6};
  checkSliceIterator<0>(shape);
  checkSliceIterator<1>(shape);
  checkSliceIterator<2>(shape);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
