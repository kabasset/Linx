// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE CreationExample

#include "Linx/Base/Random.h"
#include "Linx/Data/Image.h"
#include "Linx/Data/Sequence.h"
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

BOOST_AUTO_TEST_CASE(resize_test)
{
  //! [resize]
  auto a = Linx::Sequence("deduced static-size", {1, 2, 3, 4});
  auto b = Linx::resize<5>("padded, static-size", {1, 2, 3, 4});
  auto c = Linx::resize(3, "cropped, dynamic-size", {1, 2, 3, 4});
  auto d = Linx::resize<4, Kokkos::HostSpace>("static-size, on host", {1, 2, 3, 4});
  //! [resize]

  ASSERT(a.n == 4);
  ASSERT(b.n == 5);
  ASSERT(c.n == -1);
  ASSERT(c.size() == 3);
  ASSERT(d.n == 4);

  const auto& a_on_host = Linx::on_host(a);
  ASSERT(a_on_host.front() == 1);
  ASSERT(a_on_host.back() == 4);

  const auto& b_on_host = Linx::on_host(b);
  ASSERT(b_on_host[0] == 1);
  ASSERT(b_on_host[3] == 4);
  ASSERT(b_on_host[4] == 0);

  const auto& c_on_host = Linx::on_host(c);
  ASSERT(c_on_host.front() == 1);
  ASSERT(c_on_host.back() == 3);

  ASSERT(d.front() == 1);
  ASSERT(d.back() == 4);
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
  auto v = std::vector {1, 2, 3, 4, 5, 6};
  auto a = Linx::Raster<int, 2>(Linx::Wrap(v.data()), 3, 2);
  a.pow(2);

  ASSERT(a(2, 0) == 9);
  ASSERT(v[2] == 9);
  //! [wrap]
}

BOOST_AUTO_TEST_CASE(builtins_test)
{
  //! [builtins]
  // Uniform arrays
  auto a = Linx::fill<12>("static sequence", 3.14);
  auto b = Linx::fill(12, "dynamic sequence", 3.14);
  auto c = Linx::fill("2D image", 3.14, 4, 3);

  // {0, 30, 60, ... , 330}
  auto d = Linx::range<12>("static sequence", 0, 30);
  auto e = Linx::range(12, "dynamic sequence", 0, 30);
  auto f = Linx::linspace<12>("static sequence", Linx::Slice(0, 360));
  auto g = Linx::linspace(12, "dynamic sequence", Linx::Slice(0, 360));
  auto h = Linx::linspace<12, Kokkos::HostSpace>("static host sequence", Linx::Slice(0, 360));
  //! [builtins]

  ASSERT(a.size() == 12);
  ASSERT(a.contains_only(3.14));
  ASSERT(b.size() == 12);
  ASSERT(b.contains_only(3.14));
  ASSERT(c.n == 2);
  ASSERT(c.size() == 12);
  ASSERT(c.contains_only(3.14));

  ASSERT(d.size() == 12);
  ASSERT(e.size() == 12);
  ASSERT(f.size() == 12);
  ASSERT(g.size() == 12);

  ASSERT(h.size() == 12);
}

BOOST_AUTO_TEST_CASE(generators_test)
{
  //! [generators]
  // Generate from indices
  auto a = Linx::generate<12>("static sequence of squares", KOKKOS_LAMBDA(auto i) { return i * i; });
  auto b = Linx::generate("2D image of row-major indices", KOKKOS_LAMBDA(int i, int j) { return i * j; }, 4, 3);

  // Generate random numbers
  auto c = Linx::generate(12, "dynamic random sequence", Linx::UniformRng(Linx::Slice(0., 1.)));
  auto d = Linx::generate("2D random image", Linx::GaussianRng({100, 15}, 42), 4, 3);

  // Generate from other arrays
  auto e = Linx::generate("sum of squares", KOKKOS_LAMBDA(int a_i, double c_i) { return a_i + c_i * c_i; }, a, c);
  //! [generators]
}

BOOST_AUTO_TEST_SUITE_END()
