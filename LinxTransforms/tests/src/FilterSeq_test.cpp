// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Transforms/FilterLib.h"
#include "Linx/Transforms/Interpolation.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(FilterSeq_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(even_window_1d_test)
{
  auto f0 = correlation_along<int, 0>({0, 1});
  const auto& w0 = f0.window();
  BOOST_TEST(w0 == (Box<1> {{-1}, {0}}));
  auto f1 = correlation_along<int, 1>({2, 3});
  const auto& w1 = f1.window();
  BOOST_TEST(w1 == (Box<2> {{0, -1}, {0, 0}}));
}

BOOST_AUTO_TEST_CASE(odd_window_1d_test)
{
  auto f0 = correlation_along<int, 0>({0, 1, 2});
  const auto& w0 = f0.window();
  BOOST_TEST(w0 == (Box<1> {{-1}, {1}}));
  auto f1 = correlation_along<int, 1>({3, 4, 5});
  const auto& w1 = f1.window();
  BOOST_TEST(w1 == (Box<2> {{0, -1}, {0, 1}}));
}

BOOST_AUTO_TEST_CASE(odd_window_2d_test)
{
  auto f23 = correlation_along<int, 2, 3>({6, 7, 8});
  const auto& f2 = f23.filter<0>();
  const auto& w2 = f2.window();
  std::cout << "w2: " << w2.front() << " - " << w2.back() << std::endl;
  BOOST_TEST(w2 == (Box<3> {{0, 0, -1}, {0, 0, 1}}));
  const auto& f3 = f23.filter<1>();
  const auto& w3 = f3.window();
  std::cout << "w3: " << w3.front() << " - " << w3.back() << std::endl;
  BOOST_TEST(w3 == (Box<4> {{0, 0, 0, -1}, {0, 0, 0, 1}}));
  const auto& w23 = f23.window();
  std::cout << "w23: " << w23.front() << " - " << w23.back() << std::endl;
  BOOST_TEST(w23 == (Box<4> {{0, 0, -1, -1}, {0, 0, 1, 1}}));
}

BOOST_AUTO_TEST_CASE(prewitt_inc_test)
{
  Raster<int> expected({3, 3}, {1, 0, -1, 1, 0, -1, 1, 0, -1});
  BOOST_TEST((prewitt_filter<int, 0, 1>().impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(prewitt_dec_test)
{
  Raster<int> expected({3, 3}, {-1, 0, 1, -1, 0, 1, -1, 0, 1});
  BOOST_TEST((prewitt_filter<int, 0, 1>(-1).impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(sobel_inc_test)
{
  Raster<int> expected({3, 3}, {1, 0, -1, 2, 0, -2, 1, 0, -1});
  BOOST_TEST((sobel_filter<int, 0, 1>().impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(sobel_dec_test)
{
  Raster<int> expected({3, 3}, {-1, 0, 1, -2, 0, 2, -1, 0, 1});
  BOOST_TEST((sobel_filter<int, 0, 1>(-1).impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(scharr_inc_test)
{
  Raster<int> expected({3, 3}, {3, 0, -3, 10, 0, -10, 3, 0, -3});
  BOOST_TEST((scharr_filter<int, 0, 1>().impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(scharr_dec_test)
{
  Raster<int> expected({3, 3}, {-3, 0, 3, -10, 0, 10, -3, 0, 3});
  BOOST_TEST((scharr_filter<int, 0, 1>(-1).impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(laplacian_plus_test)
{
  Raster<int> expected({3, 3}, {0, 1, 0, 1, -2, 1, 0, 1, 0});
  BOOST_TEST((laplacian_filter<int, 0, 1>().impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(laplacian_minus_test)
{
  Raster<int> expected({3, 3}, {0, -1, 0, -1, 2, -1, 0, -1, 0});
  BOOST_TEST((laplacian_filter<int, 0, 1>(-1).impulse()) == expected);
}

BOOST_AUTO_TEST_CASE(orthogonal_associativity_commutativity_test)
{
  const auto a = correlation_along<int, 0>({1, 0, -1});
  std::cout << "a: " << a.window().front() << " - " << a.window().back() << std::endl;
  const auto b = correlation_along<int, 1>({1, 2, 3});
  std::cout << "b: " << b.window().front() << " - " << b.window().back() << std::endl;
  const auto c = a * b;
  const auto wtmp = c.window();
  std::cout << "c: " << wtmp.front() << " - " << wtmp.back() << std::endl;
  const auto raster = Raster<int>({3, 3}).range();
  std::cout << "raster: " << raster << std::endl;
  const auto direct = c * extrapolation(raster, 0);
  std::cout << "direct: " << direct << std::endl;
  const auto associated = a * b * extrapolation(raster, 0);
  std::cout << "associated: " << associated << std::endl;
  const auto commutated = b * a * extrapolation(raster, 0);
  std::cout << "commutated: " << commutated << std::endl;
  BOOST_TEST(associated == direct);
  BOOST_TEST(commutated == direct);
}

// BOOST_AUTO_TEST_CASE(sum3x3_dirichlet_test)
// {
//   const SeparableKernel<int, 0, 1, 2> kernel({1, 1, 1});
//   const auto raster = Raster<int, 3>({3, 3, 3}).fill(1);
//   const auto sum = kernel * extrapolation(raster, 0);
//   const std::vector<int> expected {
//       8,  12, 8,  12, 18, 12, 8,  12, 8, // z = 0
//       12, 18, 12, 18, 27, 18, 12, 18, 12, // z = 1
//       8,  12, 8,  12, 18, 12, 8,  12, 8}; // z = 2
//   for (std::size_t i = 0; i < 3 * 3 * 3; ++i) {
//     BOOST_TEST(sum[i] == expected[i]);
//   }
// }

// BOOST_AUTO_TEST_CASE(sum3x3_neumann_test)
// {
//   const SeparableKernel<int, 0, 1, 2> kernel({1, 1, 1});
//   const auto raster = Raster<int, 3>({3, 3, 3}).fill(1);
//   const auto sum = kernel * extrapolation<NearestNeighbor>(raster);
//   const std::vector<int> expected(raster.size(), kernel.window().size());
//   for (std::size_t i = 0; i < 3 * 3 * 3; ++i) {
//     BOOST_TEST(sum[i] == expected[i]);
//   }
// }

// BOOST_AUTO_TEST_CASE(sobel_test)
// {
//   const auto sobel0 = SeparableKernel<int, 0, 1>::sobel();
//   const auto sobel1 = SeparableKernel<int, 1, 0>::sobel(-1);
//   const auto raster = Raster<int, 3>({3, 3, 3}).fill(1);
//   const auto edges0 = sobel0 * extrapolation(raster, 0);
//   const auto edges1 = sobel1 * extrapolation(raster, 0);
//   const std::vector<int> expected0 {
//       3, 0, -3, 4, 0, -4, 3, 0, -3, // z = 0
//       3, 0, -3, 4, 0, -4, 3, 0, -3, // z = 1
//       3, 0, -3, 4, 0, -4, 3, 0, -3}; // z = 2
//   const std::vector<int> expected1 {
//       -3, -4, -3, 0, 0, 0, 3, 4, 3, // z = 0
//       -3, -4, -3, 0, 0, 0, 3, 4, 3, // z = 1
//       -3, -4, -3, 0, 0, 0, 3, 4, 3}; // z = 2
//   BOOST_TEST(edges0.container() == expected0);
//   BOOST_TEST(edges1.container() == expected1);
// }

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
