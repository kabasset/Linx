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

namespace Impl {

/*
int main(int argc, const char* argv[])
{
  //! [context]
  // Initialize Kokkos
  Linx::ProgramContext context("Some program description", argc, argv);
  
  // Declare specific program options
  context.positional("input", "The input filename");
  context.positional("output", "The output filename", "/tmp/out.fits");
  context.named("iter,n", "The Number of iterations", 4);
  context.flag("verbose,v", "Enable verbose logging");
  
  // Parse options
  context.parse();
  
  // Use values
  const auto input = context["input"];
  const auto output = context["output"];
  const auto iter_count = context.as<int>("iter");
  const auto verbose = context.has("verbose");
  
  // Example command line: program in.fits -n6 -v
  //! [context]
  
  return 0;
}
*/ // FIXME to dedicated example program

} // namespace Impl

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(basics_test)
{
  //! [basics]
  auto a = Linx::Sequence<int, 42>("sequence on device").arithmetic(14, 3);
  ASSERT(a.matches(KOKKOS_LAMBDA(int i) { return 14 + 3 * i; }));

  auto b = Linx::Image<double, 3>("image on device", 16, 9, 3).fill_with_offsets_from_data();
  const auto& b_on_host = Linx::on_host(b);
  ASSERT(b_on_host(0, 0, 0) == 0);
  ASSERT(b_on_host(15, 8, 2) == b.size() - 1);

  auto c = Linx::Raster<double, 3>("image on host", 16, 9, 3).fill_with_offsets_from_data();
  ASSERT(c.matches(KOKKOS_LAMBDA(int i, int j, int k) { return i + j * 16 + k * 16 * 9; }));
  //! [basics]
}

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
  auto a = Linx::generate<10>("noise", Linx::PoissonRng(20.));

  auto manual = Linx::same_layout("sqrt(noise)", a);
  Linx::for_each("sqrt", a.domain(), KOKKOS_LAMBDA(int i) { manual(i) = Kokkos::sqrt(a(i)); });

  auto functional = Linx::generate("sqrt", Linx::Sqrt(), a);

  auto builtin = Linx::sqrt(a);

  ASSERT(manual == builtin);
  ASSERT(functional == builtin);
  //! [array_access_transform]
}

BOOST_AUTO_TEST_CASE(label_test)
{
  //! [label]
  auto x = Linx::arithmetic<10>("x", Linx::Slice(0, std::numbers::pi));
  auto y = Linx::sin(x);
  auto z = Linx::pow(y, 2);
  ASSERT(y.label() == "sin(x)");
  ASSERT(z.label() == "pow(sin(x), 2)");
  //! [label]
}

BOOST_AUTO_TEST_CASE(functional_test)
{
  // FIXME update clang-format to interpret macro KOKKOS_LAMBDA and remove spurious parentheses
  //! [functional]
  auto a = Linx::arithmetic<4>("a"); // {0, 1, 2, 3}
  a -= (KOKKOS_LAMBDA(int i) { return i; }); // {0, 0, 0, 0}

  ASSERT(a.contains_only(0));

  auto b = Linx::fill<4>("b", 2); // {2, 2, 2, 2}
  b.pow(KOKKOS_LAMBDA(int i) { return i; }); // {1, 2, 4, 8}

  ASSERT(b == Linx::geometric<4>("expected", 1, 2));
  //! [functional]
}

BOOST_AUTO_TEST_SUITE_END()
