// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE CreationExample

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

#define ASSERT(...) BOOST_TEST((__VA_ARGS__))

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(default_test)
{
  //! [default]
  auto a = Linx::Sequence<int, 10>("default sequence");
  ASSERT(a.size() == 10);
  ASSERT(a.contains_only(0));

  auto b = Linx::Image<int, 2>("default image", 4, 3);
  ASSERT(b.size() == 12);
  ASSERT(b.contains_only(0));
  //! [default]
}

BOOST_AUTO_TEST_CASE(rowwise_test)
{
  //! [rowwise]
  auto a = Linx::rowwise("1D int", {1, 2, 3, 4});

  ASSERT(a.n == 1);
  ASSERT(typeid(a.front()) == typeid(int));

  auto b = Linx::rowwise("2D char", {{'a', 'b'}, {'c', 'd'}});

  ASSERT(b.n == 2);
  ASSERT(typeid(b.front()) == typeid(char));

  auto c = Linx::rowwise<float>("3D float", {{{1, 2, 3}, {4, 5, 6}}});

  ASSERT(c.n == 3);
  ASSERT(typeid(c.front()) == typeid(float));
  //! [rowwise]

  static_assert(std::is_same_v<decltype(a)::element_type, int>);
  ASSERT(a.label() == "1D int");
  ASSERT(a.size() == 4);
  const auto& a_on_host = Linx::on_host(a);
  ASSERT(a_on_host(0) == 1);
  ASSERT(a_on_host(1) == 2);
  ASSERT(a_on_host(2) == 3);
  ASSERT(a_on_host(3) == 4);

  static_assert(std::is_same_v<decltype(b)::element_type, char>);
  ASSERT(b.label() == "2D char");
  ASSERT(b.size() == 4);
  ASSERT(b.extent(0) == 2);
  ASSERT(b.extent(1) == 2);
  const auto& b_on_host = Linx::on_host(b);
  for (int j = 0; j < 2; ++j) {
    for (int i = 0; i < 2; ++i) {
      ASSERT(b_on_host(i, j) == char('a' + 2 * j + i));
    }
  }

  static_assert(std::is_same_v<decltype(c)::element_type, float>);
  ASSERT(c.label() == "3D float");
  ASSERT(c.size() == 6);
  ASSERT(c.extent(0) == 3);
  ASSERT(c.extent(1) == 2);
  ASSERT(c.extent(2) == 1);
  const auto& c_on_host = Linx::on_host(c);
  for (int j = 0; j < 2; ++j) {
    for (int i = 0; i < 3; ++i) {
      ASSERT(c_on_host(i, j, 0) == float(1 + 3 * j + i));
    }
  }
}

BOOST_AUTO_TEST_CASE(wrap_test)
{
  //! [wrap]
  //! [wrap]
}

BOOST_AUTO_TEST_CASE(copy_test)
{
  //! [copy]
  //! [copy]
}

BOOST_AUTO_TEST_CASE(builtins_test)
{
  //! [builtins]
  //! [builtins]
}

BOOST_AUTO_TEST_CASE(generators_test)
{
  //! [generators]
  //! [generators]
}

BOOST_AUTO_TEST_SUITE_END()
