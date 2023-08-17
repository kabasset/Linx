// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Transforms/Interpolation.h"
#include "Linx/Transforms/SeparableKernel.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(SeparableKernel_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(composition_test)
{
  const auto kernel = SeparableKernel<int, 0, 1>::sobel(-1).compose();
  std::vector<int> values {1, 0, -1, 2, 0, -2, 1, 0, -1};
  Raster<int, 2> expected({3, 3}, std::move(values));
  BOOST_TEST(kernel.raster() == expected);
}

BOOST_AUTO_TEST_CASE(orthogonal_associativity_commutativity_test)
{
  const auto a = OrientedKernel<int, 0>({1, 0, -1});
  const auto b = OrientedKernel<int, 1>({1, 2, 3});
  const auto c = a * b;
  const auto raster = Raster<int>({3, 3}).range();
  const auto direct = c * extrapolate(raster, 0);
  const auto associated = a * b * extrapolate(raster, 0);
  const auto commutated = b * a * extrapolate(raster, 0);
  BOOST_TEST(associated == direct);
  BOOST_TEST(commutated == direct);
}

BOOST_AUTO_TEST_CASE(sum3x3_dirichlet_test)
{
  const SeparableKernel<int, 0, 1, 2> kernel({1, 1, 1});
  const auto raster = Raster<int, 3>({3, 3, 3}).fill(1);
  const auto sum = kernel * extrapolate(raster, 0);
  const std::vector<int> expected {
      8,  12, 8,  12, 18, 12, 8,  12, 8, // z = 0
      12, 18, 12, 18, 27, 18, 12, 18, 12, // z = 1
      8,  12, 8,  12, 18, 12, 8,  12, 8}; // z = 2
  for (std::size_t i = 0; i < 3 * 3 * 3; ++i) {
    BOOST_TEST(sum[i] == expected[i]);
  }
}

BOOST_AUTO_TEST_CASE(sum3x3_neumann_test)
{
  const SeparableKernel<int, 0, 1, 2> kernel({1, 1, 1});
  const auto raster = Raster<int, 3>({3, 3, 3}).fill(1);
  const auto sum = kernel * extrapolate<NearestNeighbor>(raster);
  const std::vector<int> expected(raster.size(), kernel.window().size());
  for (std::size_t i = 0; i < 3 * 3 * 3; ++i) {
    BOOST_TEST(sum[i] == expected[i]);
  }
}

BOOST_AUTO_TEST_CASE(sobel_test)
{
  const auto sobel0 = SeparableKernel<int, 0, 1>::sobel();
  const auto sobel1 = SeparableKernel<int, 1, 0>::sobel(-1);
  const auto raster = Raster<int, 3>({3, 3, 3}).fill(1);
  const auto edges0 = sobel0 * extrapolate(raster, 0);
  const auto edges1 = sobel1 * extrapolate(raster, 0);
  const std::vector<int> expected0 {
      3, 0, -3, 4, 0, -4, 3, 0, -3, // z = 0
      3, 0, -3, 4, 0, -4, 3, 0, -3, // z = 1
      3, 0, -3, 4, 0, -4, 3, 0, -3}; // z = 2
  const std::vector<int> expected1 {
      -3, -4, -3, 0, 0, 0, 3, 4, 3, // z = 0
      -3, -4, -3, 0, 0, 0, 3, 4, 3, // z = 1
      -3, -4, -3, 0, 0, 0, 3, 4, 3}; // z = 2
  BOOST_TEST(edges0.container() == expected0);
  BOOST_TEST(edges1.container() == expected1);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
