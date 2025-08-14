// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ReductionExample

#include "Linx/Data/Sequence.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

#define ASSERT(...) BOOST_TEST((__VA_ARGS__))

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(single_test)
{
  //! [single]
  auto a = Linx::arithmetic<100>("a");

  // Sum all elements
  auto functor = Linx::reduce("sum", Linx::Add(), a);
  auto builtin = Linx::sum(a);

  BOOST_TEST(functor == (100 * 99) / 2);
  ASSERT(builtin == functor);
  //! [single]
}

BOOST_AUTO_TEST_CASE(transform_test)
{
  //! [transform]
  auto a = Linx::geometric<100>("a", 1, -1); // 1, -1, 1, -1, ...

  // L1 norm
  auto functor = Linx::transform_reduce(
      "norm",
      KOKKOS_LAMBDA(int i) { return Kokkos::abs(i); },
      Linx::Add(),
      a);
  auto builtin = Linx::norm<1>(a);

  BOOST_TEST(functor == 100);
  ASSERT(builtin == functor);
  //! [transform]
}

BOOST_AUTO_TEST_CASE(multiple_test)
{
  //! [multiple]
  auto a = Linx::seq("a", 1, 2, 3, 4);
  auto b = Linx::seq("b", 4, 3, 2, 1);

  // Squared L2-distance between a and b
  auto lambda = Linx::transform_reduce(
      "distance",
      KOKKOS_LAMBDA(int a_i, int b_i) { return Kokkos::pow(a_i - b_i, 2); },
      Linx::Add(),
      a,
      b);
  auto functor = Linx::transform_reduce("distance", Linx::Abspow<2>(), Linx::Add(), a, b);
  auto builtin = Linx::distance(a, b);

  // Third array, of doubles
  auto c = Linx::seq("c", 3.0e8, -1.0, -1.0, -1.0);

  // Mixing element types and function types
  auto mixed = Linx::transform_reduce(
      "interval",
      KOKKOS_LAMBDA(double c_i, int a_i, int b_i) { return c_i * Kokkos::pow(a_i - b_i, 2); },
      Linx::Add(),
      c,
      a,
      b);
  //! [multiple]

  ASSERT(lambda == 20);
  ASSERT(functor == 20);
  ASSERT(builtin == 20);
  BOOST_TEST(mixed == 9 * 3.0e8 - 1 - 1 - 9); // FIXME!!!
}

BOOST_AUTO_TEST_SUITE_END()
