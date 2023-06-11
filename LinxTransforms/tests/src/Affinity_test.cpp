// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Transforms/Affinity.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Affinity_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(translation_test) {
  const Vector<double, 3> vector {0, 1, 2};
  const auto translation = Affinity<3>::translation(vector);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = translation[in];
  const auto expected = in + vector;
  BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(scaling_origin_test) {
  const Vector<double, 3> vector {0, 1, 2};
  const auto scaling = Affinity<3>::scaling(vector);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = scaling[in];
  auto expected = in;
  for (Index i = 0; i < 3; ++i) {
    expected[i] *= vector[i];
  }
  BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(rotation_origin_90z_test) {
  const auto rotation = Affinity<3>::rotationDegrees(90, 0, 1);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation[in];
  const Vector<double, 3> expected {-4, 3, 5};
  BOOST_TEST(out == expected);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
