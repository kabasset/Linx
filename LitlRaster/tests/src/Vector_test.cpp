// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Vector.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Vector_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(vector_init_test) {

  const std::vector<Index> indices {1, 2, 3};

  Position<-1> list {indices[0], indices[1], indices[2]};
  BOOST_TEST(list.size() == indices.size());

  Position<-1> iterable(indices);
  BOOST_TEST(iterable.size() == indices.size());

  Position<-1> dimension(indices.size());
  for (std::size_t i = 0; i < indices.size(); ++i) {
    dimension[i] = indices[i];
  }

  for (std::size_t i = 0; i < indices.size(); ++i) {
    BOOST_TEST(list[i] == indices[i]);
    BOOST_TEST(iterable[i] == indices[i]);
    BOOST_TEST(dimension[i] == indices[i]);
  }
}

BOOST_AUTO_TEST_CASE(array_init_test) {

  const std::array<Index, 3> indices {1, 2, 3};

  Position<3> list {indices[0], indices[1], indices[2]};
  BOOST_TEST(list.size() == indices.size());

  Position<-1> iterable(indices);
  BOOST_TEST(iterable.size() == indices.size());

  Position<3> dimension(indices.size());
  for (std::size_t i = 0; i < indices.size(); ++i) {
    dimension[i] = indices[i];
  }

  for (std::size_t i = 0; i < indices.size(); ++i) {
    BOOST_TEST(list[i] == indices[i]);
    BOOST_TEST(iterable[i] == indices[i]);
    BOOST_TEST(dimension[i] == indices[i]);
  }
}

BOOST_AUTO_TEST_CASE(arithmetics_test) {

  const Position<4> indices {0, 1, 2, 3};
  const Position<4> evens {0, 2, 4, 6};
  const Position<4> positives {1, 2, 3, 4};

  const auto plus = indices + indices;
  BOOST_TEST(plus == evens);
  const auto minus = plus - indices;
  BOOST_TEST(minus == indices);

  const auto multiplies = indices * 2;
  BOOST_TEST(multiplies == evens);
  const auto divides = multiplies / 2;
  BOOST_TEST(divides == indices);

  const auto inc = indices + 1;
  BOOST_TEST(inc == positives);
  const auto dec = inc - 1;
  BOOST_TEST(dec == indices);
}

BOOST_AUTO_TEST_CASE(norm_test) {

  Position<3> zero {0, 0, 0};
  Position<3> x {-4, 0, 0};
  Position<3> y {0, -4, 0};
  Position<3> z {0, 0, -4};
  Position<3> xy {-4, -4, 0};
  Position<3> xyz {-4, -4, -4};

  BOOST_TEST(zero.template norm<0>() == 0);
  BOOST_TEST(x.template norm<0>() == 1);
  BOOST_TEST(y.template norm<0>() == 1);
  BOOST_TEST(z.template norm<0>() == 1);
  BOOST_TEST(xy.template norm<0>() == 2);
  BOOST_TEST(xyz.template norm<0>() == 3);
  BOOST_TEST(xyz.template distance<0>(x) == 2);
  BOOST_TEST(xyz.template distance<0>(-x) == 3);

  BOOST_TEST(zero.template norm<1>() == 0);
  BOOST_TEST(x.template norm<1>() == 4);
  BOOST_TEST(y.template norm<1>() == 4);
  BOOST_TEST(z.template norm<1>() == 4);
  BOOST_TEST(xy.template norm<1>() == 8);
  BOOST_TEST(xyz.template norm<1>() == 12);
  BOOST_TEST(xyz.template distance<1>(x) == 8);
  BOOST_TEST(xyz.template distance<1>(-x) == 16);

  BOOST_TEST(zero.template norm<2>() == 0);
  BOOST_TEST(x.template norm<2>() == 16);
  BOOST_TEST(y.template norm<2>() == 16);
  BOOST_TEST(z.template norm<2>() == 16);
  BOOST_TEST(xy.template norm<2>() == 32);
  BOOST_TEST(xyz.template norm<2>() == 48);
  BOOST_TEST(xyz.template distance<2>(x) == 32);
  BOOST_TEST(xyz.template distance<2>(-x) == 96);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
