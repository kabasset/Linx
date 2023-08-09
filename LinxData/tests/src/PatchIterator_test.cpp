// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Data/Box.h"
#include "Linx/Data/Grid.h"
#include "Linx/Data/Mask.h"
#include "Linx/Data/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(PatchIterator_test)

//-----------------------------------------------------------------------------

template <Index N, typename TRegion>
void check_iterator(const Position<N>& shape, const TRegion& region) {

  const auto raster = Raster<float, N>(shape).range();

  std::vector<float> expected;
  expected.reserve(region.size());
  for (const auto& p : region) {
    expected.push_back(raster[p]);
  }

  std::vector<float> out;
  out.reserve(expected.size());
  for (const auto& v : raster.patch(region)) {
    out.push_back(v);
  }

  BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(inner_cube_iterator_test) {
  const Position<3> shape {4, 5, 6};
  const Box<3> box {{1, 1, 1}, {2, 3, 4}};
  check_iterator(shape, box);
  check_iterator(shape, Grid<3>(box, Position<3>::one()));
  check_iterator(shape, Mask<3>(box, true));
  check_iterator(shape, Sequence<Position<3>>(box));
}

template <Index I, Index N>
void check_slice_iterator(const Position<N>& shape) {
  Line<I, 3> slice(Position<N>::one(), shape[I] - 1, 2);
  check_iterator(shape, slice);
}

BOOST_AUTO_TEST_CASE(slice_iterator_test) {
  const Position<3> shape {4, 5, 6};
  check_slice_iterator<0>(shape);
  check_slice_iterator<1>(shape);
  check_slice_iterator<2>(shape);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
