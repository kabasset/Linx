// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Transforms/Affinity.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Affinity_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(translation_test)
{
  const Vector<double, 3> vector {0, 1, 2};
  const auto translation = Affinity<3>::translation(vector);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = translation(in);
  const auto expected = in + vector;
  BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(scaling_origin_test)
{
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

BOOST_AUTO_TEST_CASE(rotation_origin_90x_test)
{
  const auto rotation = Affinity<3>::rotation_degrees(90, 1, 2);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation(in);
  const Vector<double, 3> expected {3, -5, 4};
  BOOST_TEST(out == expected, boost::test_tools::tolerance(1.e-6) << boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(rotation_origin_90y_test)
{
  const auto rotation = Affinity<3>::rotation_degrees(90, 2, 0);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation(in);
  const Vector<double, 3> expected {5, 4, -3};
  BOOST_TEST(out == expected, boost::test_tools::tolerance(1.e-6) << boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(rotation_origin_90z_test)
{
  const auto rotation = Affinity<3>::rotation_degrees(90, 0, 1);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation(in);
  const Vector<double, 3> expected {-4, 3, 5};
  BOOST_TEST(out == expected, boost::test_tools::tolerance(1.e-6) << boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(rotation_origin_270z_test)
{
  const auto rotation = Affinity<3>::rotation_degrees(90, 1, 0);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation(in);
  const Vector<double, 3> expected {4, -3, 5};
  BOOST_TEST(out == expected, boost::test_tools::tolerance(1.e-6) << boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(rotation_int_90z_test)
{
  const Vector<double, 3> center = {0, 1, 2};
  const auto rotation = Affinity<3>::rotation_degrees(90, 0, 1, center);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation(in);
  const Vector<double, 3> expected {-3, 4, 5};
  BOOST_TEST(out == expected, boost::test_tools::tolerance(1.e-6) << boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(rotation_float_90z_test)
{
  const Vector<double, 3> center = {1.5, 1.5, 0};
  const auto rotation = Affinity<3>::rotation_degrees(90, 0, 1, center);
  const Vector<double, 3> in {3, 4, 5};
  const auto out = rotation(in);
  const Vector<double, 3> expected {-1, 3, 5};
  BOOST_TEST(out == expected, boost::test_tools::tolerance(1.e-6) << boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(raster_scaling_center_2_test)
{
  const auto in = Raster<Index, 4>({3, 3, 3, 3}).range();
  const auto scaling = Affinity<4>::scaling(2, {1, 1, 1, 1});
  const auto out = scaling.warp<Cubic>(in);
  const auto out2 = scale<Cubic>(in, 2);
  const auto out_nn = interpolation<NearestNeighbor>(out);
  const auto out2_nn = interpolation<NearestNeighbor>(out2);
  for (const auto& p : in.domain()) {
    const auto q = scaling(p);
    if (in.contains(q)) {
      BOOST_TEST(out_nn(q) == in[p]);
      BOOST_TEST(out2_nn(q) == in[p]);
    }
  }
}

BOOST_AUTO_TEST_CASE(raster_rotation_center_90z_test)
{
  const auto in = Raster<Index>({4, 4}).range();
  const auto rotation = Affinity<2>::rotation_degrees(90, 0, 1, {1.5, 1.5});
  const auto out = rotation.warp<NearestNeighbor>(in);
  const auto out2 = rotate_degrees<NearestNeighbor>(in, 90);
  const auto out_nn = interpolation<NearestNeighbor>(out);
  const auto out2_nn = interpolation<NearestNeighbor>(out2);
  for (const auto& p : in.domain()) {
    const auto q = rotation(p);
    if (in.contains(q)) {
      BOOST_TEST(out_nn(q) == in[p]);
      BOOST_TEST(out2_nn(q) == in[p]);
    }
  }
}

BOOST_AUTO_TEST_CASE(patch_rotation_center_30z_test)
{
  const auto in = Raster<Index>({5, 5}).range();
  const auto interpolator = interpolation<Linear>(in);
  const auto rotation = Affinity<2>::rotation_degrees(30, 0, 1, {2, 2});
  const auto inv = inverse(rotation);
  Raster<double> out(in.shape());
  auto patch = out(Box<2>({1, 1}, {3, 3}));
  rotation.transform(interpolator, patch);
  for (const auto& p : out.domain()) {
    if (patch.domain().contains(p)) {
      BOOST_TEST(out[p] == (interpolator(inv(p))));
    } else {
      BOOST_TEST(out[p] == 0);
    }
  }
}

BOOST_AUTO_TEST_CASE(raster_upsampling_sesquiple_test)
{
  const auto in = Raster<float>({3, 2}).range();
  const auto out = upsample<NearestNeighbor>(in, 1.5);
  BOOST_TEST(out.shape() == in.shape() * 1.5);
  for (const auto& p : out.domain()) {
    Vector<double> q(p);
    q += 0.75;
    q /= 1.5;
    Position<> r(q);
    BOOST_TEST(out[p] == in[r]);
  }
}

BOOST_AUTO_TEST_CASE(raster_upsampling_double_test)
{
  const auto in = Raster<float>({3, 2}).range();
  const auto out = upsample<NearestNeighbor>(in, 2); // FIXME out of bounds somehow
  BOOST_TEST(out.shape() == in.shape() * 2);
  for (const auto& p : out.domain()) {
    Vector<double> q(p);
    q += 1;
    q /= 2;
    Position<> r(q);
    BOOST_TEST(out[p] == in[r]);
  }
}

BOOST_AUTO_TEST_CASE(raster_upsampling_triple_test)
{
  const auto in = Raster<float>({3, 2}).range();
  const auto out = upsample<NearestNeighbor>(in, 3);
  BOOST_TEST(out.shape() == in.shape() * 3);
  for (const auto& p : out.domain()) {
    Vector<double> q(p);
    q += 1.5;
    q /= 3;
    Position<> r(q);
    BOOST_TEST(out[p] == in[r]);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
