// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/Ball.h"
#include "LitlRaster/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(BallIterator_test)

//-----------------------------------------------------------------------------

template <Index N>
double distanceL0(const Position<N>& p, const Position<N>& q) {
  double out = 0;
  for (Index i = 0; i < N; ++i) {
    out += double(p[i] == q[i]);
  }
  return out;
}

template <Index N>
double distanceL1(const Position<N>& p, const Position<N>& q) {
  double out = 0;
  for (Index i = 0; i < N; ++i) {
    out += std::abs(p[i] - q[i]);
  }
  return out;
}

template <Index N>
double distanceL2(const Position<N>& p, const Position<N>& q) {
  double out = 0;
  for (Index i = 0; i < N; ++i) {
    out += (p[i] - q[i]) * (p[i] - q[i]);
  }
  return out;
}

// BOOST_AUTO_TEST_CASE(l0_test) {
//   const double radius = 2;
//   const Position<3> center {1, 2, 3};
//   Ball<3, 0> ball(radius, center);
//   std::vector<Position<3>> out;
//   std::vector<Position<3>> expected;
//   for (const auto& p : ball.box()) {
//     if (distanceL0(p, center) <= 1) {
//       expected.push_back(p);
//     }
//   }
//   for (const auto& p : ball) {
//     out.push_back(p);
//   }
//   BOOST_TEST(out == expected);
// }

BOOST_AUTO_TEST_CASE(l1_test) {
  const double radius = 2;
  const Position<3> center {1, 2, 3};
  Ball<3, 1> ball(radius, center);
  std::vector<Position<3>> out;
  std::vector<Position<3>> expected;
  for (const auto& p : ball.box()) {
    if (distanceL1(p, center) <= radius) {
      expected.push_back(p);
    }
  }
  for (const auto& p : ball) {
    out.push_back(p);
  }
  BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(l2_test) {
  const double radius = 2;
  const Position<3> center {1, 2, 3};
  Ball<3> ball(radius, center);
  std::vector<Position<3>> out;
  std::vector<Position<3>> expected;
  for (const auto& p : ball.box()) {
    if (distanceL2(p, center) <= radius * radius) {
      expected.push_back(p);
    }
  }
  for (const auto& p : ball) {
    out.push_back(p);
  }
  BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(domain_and_shift_test) {
  const double radius = 2;
  const Position<3> center {1, 2, 3};
  Ball<3> ball(radius);
  std::vector<Position<3>> expected;
  auto out = ball.domain();
  out += center;
  for (const auto& p : ball) {
    expected.push_back(p + center);
  }
  BOOST_TEST(out.size() == ball.size());
  BOOST_TEST(out.container() == expected);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
