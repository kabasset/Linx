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
  const auto out = translation(in);
  const auto expected = in + vector;
  BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(scaling_origin_test) {
  const Vector<double, 3> vector {0, 1, 2};
  const auto scaling = Affinity<3>::scaling(vector);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = scaling(in);
  auto expected = in;
  for (Index i = 0; i < 3; ++i) {
    expected[i] *= vector[i];
  }
  BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(rotation_origin_90x_test) {
  const auto rotation = Affinity<3>::rotationDegrees(90, 1, 2);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation(in);
  const Vector<double, 3> expected {3, -5, 4};
  BOOST_TEST(out == expected, boost::test_tools::tolerance(1.e-6) << boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(rotation_origin_90y_test) {
  const auto rotation = Affinity<3>::rotationDegrees(90, 2, 0);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation(in);
  const Vector<double, 3> expected {5, 4, -3};
  BOOST_TEST(out == expected, boost::test_tools::tolerance(1.e-6) << boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(rotation_origin_90z_test) {
  const auto rotation = Affinity<3>::rotationDegrees(90, 0, 1);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation(in);
  const Vector<double, 3> expected {-4, 3, 5};
  BOOST_TEST(out == expected, boost::test_tools::tolerance(1.e-6) << boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(rotation_origin_270z_test) {
  const auto rotation = Affinity<3>::rotationDegrees(90, 1, 0);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation(in);
  const Vector<double, 3> expected {4, -3, 5};
  BOOST_TEST(out == expected, boost::test_tools::tolerance(1.e-6) << boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(rotation_int_90z_test) {
  const Vector<double, 3> center = {0, 1, 2};
  const auto rotation = Affinity<3>::rotationDegrees(90, 0, 1, center);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation(in);
  const Vector<double, 3> expected {-3, 4, 5};
  BOOST_TEST(out == expected, boost::test_tools::tolerance(1.e-6) << boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(rotation_float_90z_test) {
  const Vector<double, 3> center = {1.5, 1.5, 0};
  const auto rotation = Affinity<3>::rotationDegrees(90, 0, 1, center);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation(in);
  const Vector<double, 3> expected {-1, 3, 5};
  BOOST_TEST(out == expected, boost::test_tools::tolerance(1.e-6) << boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(raster_rotation_center_90z_test) {
  const auto in = Raster<Index>({4, 4}).range();
  const auto interpolator = interpolate<Linx::NearestNeighbor>(in);
  const auto rotation = Affinity<2>::rotationDegrees(90, 0, 1, {1.5, 1.5});
  const auto out = rotation * interpolator;
  for (const auto& p : out.domain()) {
    BOOST_TEST(out[p] == (interpolator(rotation(p))));
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
