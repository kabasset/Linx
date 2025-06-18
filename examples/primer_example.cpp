// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ArraysExample

#include "Linx/Base/Random.h"
#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/LinearFiltering.h"

#include <boost/test/unit_test.hpp>

#define ASSERT(...) BOOST_TEST((__VA_ARGS__))

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(inplace_newinstance_test)
{
  //! [inplace_newinstance]
  auto a = Linx::fill<10>("input", 3);
  a.pow(2);
  auto b = Linx::sqrt(a);
  ASSERT(a.contains_only(9));
  ASSERT(b.contains_only(3));
  //! [inplace_newinstance]
}

BOOST_AUTO_TEST_CASE(array_access_transform_test)
{
  //! [array_access_transform]
  auto normal = Linx::generate<10>("noise", Linx::PoissonRng(20.));

  auto manual = Linx::same_layout("sqrt(noise)", normal);
  Linx::for_each("sqrt", normal.domain(), KOKKOS_LAMBDA(int i) { manual(i) = Kokkos::sqrt(normal(i)); });

  auto functional = Linx::generate("sqrt", Linx::Sqrt(), normal);

  auto builtin = Linx::sqrt(+normal);

  ASSERT(manual == builtin);
  ASSERT(functional == builtin);
  //! [array_access_transform]
}

BOOST_AUTO_TEST_CASE(label_test)
{
  //! [label]
  auto x = Linx::Sequence<double, 10>("x").linspace(0, std::numbers::pi);
  auto y = Linx::sin(x);
  auto z = Linx::pow(y, 2);
  ASSERT(y.label() == "sin(x)");
  ASSERT(z.label() == "pow(sin(x), 2)");
  //! [label]
}

BOOST_AUTO_TEST_SUITE_END()
