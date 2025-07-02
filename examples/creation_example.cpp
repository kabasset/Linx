// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE CreationExample

#include "Linx/Base/Random.h"
#include "Linx/Data/Image.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/LinearFiltering.h"

#include <boost/test/unit_test.hpp>

#define ASSERT(...) BOOST_TEST((__VA_ARGS__))

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(default_test)
{
  //! [default]
  // Basic constructors
  auto a = Linx::Sequence<int, 12>("static size, default sequence");
  auto b = Linx::Sequence<int, -1>("dynamic-size, default sequence", 12);
  auto c = Linx::Image<int, 2>("default image", 4, 3);

  ASSERT(a.n == 12 && a.size() == 12 && a.contains_only(0));
  ASSERT(b.n == -1 && b.size() == 12 && b.contains_only(0));
  ASSERT(c.n == 2 && c.size() == 12 && c.contains_only(0));
  //! [default]
}

BOOST_AUTO_TEST_CASE(resize_test)
{
  //! [resize]
  // Create sequences from lists
  auto a = Linx::Sequence("deduced static-size", {1, 2, 3, 4});
  auto b = Linx::resize<5>("padded, static-size", {1, 2, 3, 4});
  auto c = Linx::resize(3, "cropped, dynamic-size", {1, 2, 3, 4});
  auto d = Linx::resize<4, Kokkos::HostSpace>("static-size, on host", {1, 2, 3, 4});

  ASSERT(a.n == 4);
  ASSERT(b.n == 5);
  ASSERT(c.n == -1 && c.size() == 3);
  ASSERT(d.n == 4);
  //! [resize]

  ASSERT(a.matches(Linx::Add(1)));
  ASSERT(b.matches(KOKKOS_LAMBDA(int i) { return i < 4 ? i + 1 : 0; }));
  ASSERT(c.matches(Linx::Add(1)));
  ASSERT(d.matches(Linx::Add(1)));
}

BOOST_AUTO_TEST_CASE(rowwise_test)
{
  //! [rowwise]
  // Create up-to-3D images row by row
  auto a = Linx::rowwise("1D int", {1, 2, 3, 4});
  auto b = Linx::rowwise("2D char", {{'a', 'b'}, {'c', 'd'}});
  auto c = Linx::rowwise<float>("3D float", {{{1, 2, 3}, {4, 5, 6}}});

  ASSERT(a.n == 1 && typeid(a.front()) == typeid(int));
  ASSERT(b.n == 2 && typeid(b.front()) == typeid(char));
  ASSERT(c.n == 3 && typeid(c.front()) == typeid(float));
  //! [rowwise]

  static_assert(std::is_same_v<decltype(a)::element_type, int>);
  ASSERT(a.label() == "1D int");
  ASSERT(a.size() == 4);
  ASSERT(a.matches(Linx::Add(1)));

  static_assert(std::is_same_v<decltype(b)::element_type, char>);
  ASSERT(b.label() == "2D char");
  ASSERT(b.size() == 4);
  ASSERT(b.extent(0) == 2);
  ASSERT(b.extent(1) == 2);
  ASSERT(b.matches(KOKKOS_LAMBDA(int i, int j) { return char('a' + 2 * j + i); }));

  static_assert(std::is_same_v<decltype(c)::element_type, float>);
  ASSERT(c.label() == "3D float");
  ASSERT(c.size() == 6);
  ASSERT(c.extent(0) == 3);
  ASSERT(c.extent(1) == 2);
  ASSERT(c.extent(2) == 1);
  ASSERT(c.matches(KOKKOS_LAMBDA(int i, int j, int) { return float(1 + 3 * j + i); }));
}

BOOST_AUTO_TEST_CASE(wrap_test)
{
  //! [wrap]
  // Wrap a data pointer
  auto v = std::vector {1, 2, 3, 4, 5, 6};
  auto a = Linx::Raster<int, 2>(Linx::Wrap(v.data()), 3, 2);

  // Perform Linx operations
  a(2, 0) = 7;
  a.pow(2);

  // Perform std operations
  std::ranges::sort(begin(a), end(a));
  for (auto& a_i : a) {
    ++a_i;
  }

  ASSERT(v[0] == 2);
  ASSERT(v[5] == 50);
  //! [wrap]
}

BOOST_AUTO_TEST_CASE(builtins_test)
{
  //! [builtins]
  // Uniform arrays
  auto a = Linx::fill<12>("double[12]", 3.14);
  auto b = Linx::fill(12, "double*", 3.14);
  auto c = Linx::fill("double**", 3.14, 4, 3);

  // {0, 30, 60, ... , 330}
  auto d = Linx::arithmetic<12>("0, +30, ...", 0, 30);
  auto e = Linx::arithmetic(12, "0, +30, ...", 0, 30);
  auto f = Linx::arithmetic<12>("[0, ... , 360)", Linx::Slice(0, 360));
  auto g = Linx::arithmetic(12, "[0, ... , 330]", Linx::Segment<int>(0, 330));
  auto h = Linx::arithmetic<12, Kokkos::HostSpace>("on host", Linx::Slice(0, 360));
  //! [builtins]

  ASSERT(a.size() == 12 && a.contains_only(3.14));
  ASSERT(b.size() == 12 && b.contains_only(3.14));
  ASSERT(c.n == 2 && c.size() == 12 && c.contains_only(3.14));

  ASSERT(d.size() == 12);
  ASSERT(e.size() == 12);
  ASSERT(f.size() == 12);
  ASSERT(g.size() == 12);

  ASSERT(h.size() == 12);
  ASSERT(h[0] == 0);
  ASSERT(h[11] == 330);
}

BOOST_AUTO_TEST_CASE(generators_test)
{
  //! [generators]
  // Generate from indices
  auto a = Linx::generate<12>("int[12]", KOKKOS_LAMBDA(int i) { return i * i; });
  auto b = Linx::generate("int**", KOKKOS_LAMBDA(int i, int j) { return i * j; }, 4, 3);

  // Generate random numbers
  auto seed = 42;
  auto c = Linx::generate(12, "double*", Linx::UniformRng(Linx::Slice(0., 1.), seed));
  auto d = Linx::generate("int***", Linx::GaussianRng({100, 15}, seed), 4, 3, 2);

  // Generate from other arrays
  auto e = Linx::generate("double[12]", KOKKOS_LAMBDA(int a_i, double c_i) { return a_i + c_i * c_i; }, a, c);
  //! [generators]

  ASSERT(a.n == 12);
  ASSERT(a.matches(Linx::Abspow<2>()));
}

BOOST_AUTO_TEST_CASE(result_test)
{
  //! [result]
  auto seed = 42;
  auto a = Linx::generate("noise", Linx::PoissonRng(10., seed), 3, 2);

  // Math function
  auto b = Linx::sqrt(a);

  // Arithmetics
  auto c = b + Linx::GaussianRng({0., 3.}, seed);

  // Filtering
  auto d = Linx::separable_laplacian<0, 1>()(a);
  //! [result]
}

BOOST_AUTO_TEST_CASE(copy_test)
{
  //! [copy]
  auto a = Linx::fill("a", 'a', 4, 3);

  ASSERT(a.contains_only('a'));

  // Shallow copy
  auto b = a;
  ++b;

  ASSERT(b.label() == "a");
  ASSERT(a.contains_only('b'));
  ASSERT(b.contains_only('b'));

  // Deep copy
  auto c = +b;
  ++c;

  ASSERT(c.label() == "copy(a)");
  ASSERT(b.contains_only('b'));
  ASSERT(c.contains_only('c'));
  //! [copy]
}

BOOST_AUTO_TEST_CASE(slicing_test)
{
  //! [slicing]
  // 3D image
  auto cube = Linx::fill("3D", 1, 16, 9, 3);
  ASSERT(Linx::sum(cube) == 16 * 9 * 3);

  // First image row
  auto row = cube[Linx::Slice()(0)(0)]; // FIXME rm ()
  ASSERT(row.n == 1);
  ASSERT(Linx::sum(row) == 16);

  // First image column
  auto column = cube[Linx::Slice(0)()(0)];
  ASSERT(column.n == 1);
  ASSERT(Linx::sum(column) == 9);

  // First image plane
  auto plane = cube[Linx::Slice(0)];
  ASSERT(plane.n == 2);
  ASSERT(Linx::sum(plane) == 16 * 9);

  // Planes 1 and 2
  auto section = cube[Linx::Slice(1, 3)];
  ASSERT(section.n == 3);
  ASSERT(Linx::sum(section) == 16 * 9 * 2);

  // Inner cube
  auto inner_3d = cube[Linx::Slice(1, 15)(1, 8)(1, 2)];
  ASSERT(inner_3d.n == 3);
  ASSERT(Linx::sum(inner_3d) == 14 * 7 * 1);

  // Inner plane
  auto inner_2d = cube[Linx::Slice(1, 15)(1, 8)(1)];
  ASSERT(inner_2d.n == 2);
  ASSERT(Linx::sum(inner_2d) == 14 * 7);
  //! [slicing]
}

BOOST_AUTO_TEST_SUITE_END()
