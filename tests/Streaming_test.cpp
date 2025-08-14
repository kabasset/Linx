// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE SlicingTest

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>
#include <sstream>

void check_streaming(const auto& in, const std::string& expected)
{
  std::cout << in << std::endl;
  auto str = (std::stringstream() << in).str();
  BOOST_TEST(str == expected);
}

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(image_1D_test)
{
  auto a = Linx::rowwise("a", {3.14});
  check_streaming(a, "a: O ~ [1]\n  [ 3.14 ]");

  auto b = Linx::default_init<int>("", Linx::Box({-1}, {1}));
  check_streaming(b, "<Image>: [-1] ~ [1]\n  [ 0 0 ]");
}

BOOST_AUTO_TEST_CASE(image_2D_test)
{
  auto a = Linx::rowwise("a", {{1, 2}, {3, 4}});
  check_streaming(a, "a: O ~ [2, 2]\n  [ [ 1 2 ]\n    [ 3 4 ] ]");

  using namespace Linx::Literals;

  auto b = Linx::default_init<int>("b", Linx::cube<2_D>(1));
  check_streaming(b, "b: [-1, -1] ~ [2, 2]\n  [ [ 0 0 0 ]\n    [ 0 0 0 ]\n    [ 0 0 0 ] ]");
}

BOOST_AUTO_TEST_CASE(image_3D_test)
{
  auto a = Linx::rowwise("a", {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}});
  check_streaming(a, "a: O ~ [2, 2, 2]\n  [ [ [ 1 2 ]\n      [ 3 4 ] ]\n    [ [ 5 6 ]\n      [ 7 8 ] ] ]");

  using namespace Linx::Literals;

  auto b = Linx::default_init<int>("b", Linx::shape<3_D, 1>());
  check_streaming(b, "b: O ~ [1, 1, 1]\n  [ [ [ 0 ] ] ]");
}

BOOST_AUTO_TEST_CASE(image_4D_test)
{
  using namespace Linx::Literals;

  auto a = Linx::default_init<int>("a", Linx::shape<4_D, 2>());
  check_streaming(a, "a: O ~ [2, 2, 2, 2]\n  [ [ [ [ 0 ... 0 ] ] ] ]");
}

BOOST_AUTO_TEST_SUITE_END()
