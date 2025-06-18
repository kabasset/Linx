// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE PrimerExample

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
  auto in = Linx::generate<10>("noise", Linx::PoissonRng(20.));

  auto manual = Linx::same_layout("sqrt(noise)", in);
  Linx::for_each("sqrt", in.domain(), KOKKOS_LAMBDA(int i) { manual(i) = Kokkos::sqrt(in(i)); });

  auto functional = Linx::generate("sqrt", Linx::Sqrt(), in);

  auto builtin = Linx::sqrt(in);

  ASSERT(manual == builtin);
  ASSERT(functional == builtin);
  //! [array_access_transform]
}

BOOST_AUTO_TEST_CASE(label_test)
{
  //! [label]
  auto x = Linx::linspace<10>("x", Linx::Slice(0, std::numbers::pi));
  auto y = Linx::sin(x);
  auto z = Linx::pow(y, 2);
  ASSERT(y.label() == "sin(x)");
  ASSERT(z.label() == "pow(sin(x), 2)");
  std::cout << x << std::endl;
  std::cout << y << std::endl;
  //! [label]
}

BOOST_AUTO_TEST_SUITE_END()
