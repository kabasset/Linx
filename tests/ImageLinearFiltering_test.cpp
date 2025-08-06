// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ImageLinearFilteringTest

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/LinearFiltering.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(correlation_impulse_response_test)
{
  using namespace Linx::Literals;

  auto in = Linx::default_init<int>("impulse", Linx::cube<2_D, 2>());
  const auto& in_on_host = Linx::on_host(in);
  in_on_host(0, 0) = 1;
  Kokkos::deep_copy(in.container(), in_on_host.container());

  std::cout << in << std::endl;

  auto k = Linx::no_init<Kokkos::complex<double>>("kernel", 3, 3).generate_offsets(); // Unique values
  k += Kokkos::complex<double>(0, 1); // Non-null imaginary part

  std::cout << k << std::endl;

  auto out = Linx::Correlation(k)(in);

  std::cout << out << std::endl;

  auto test = Linx::default_init<Kokkos::complex<double>>("test", Linx::cube<2_D, 2>());
  BOOST_TEST((out.domain() == test.domain()));
  Linx::for_each(
      "test",
      k.domain(),
      KOKKOS_LAMBDA(int i, int j) { test(2, 2) = Kokkos::conj(k(i, j)); });
  test -= out;

  std::cout << test << std::endl;

  BOOST_TEST(test.contains_only(0));
}

BOOST_AUTO_TEST_CASE(correlation_unit_response_test)
{
  auto in = Linx::fill("unit", 1, 5, 5);

  std::cout << in << std::endl;

  auto k = Linx::no_init<int>("kernel", 3, 3).generate_offsets();

  std::cout << k << std::endl;

  auto out = Linx::Correlation(k)(in);

  std::cout << out << std::endl;

  auto test = Linx::default_init<int>("test", 5, 5);
  BOOST_TEST((out.domain() == test.domain()));
  auto sum = Linx::sum(k);
  Linx::for_each(
      "test",
      k.domain(),
      KOKKOS_LAMBDA(int i, int j) { test(2 - i, 2 - j) = sum; });
  test -= out;

  std::cout << test << std::endl;

  BOOST_TEST(test.contains_only(0));
}

BOOST_AUTO_TEST_CASE(convolution_impulse_response_test)
{
  auto in = Linx::default_init<int>("impulse", 5, 5);
  const auto& in_on_host = Linx::on_host(in);
  in_on_host(2, 2) = 1;
  Kokkos::deep_copy(in.container(), in_on_host.container());

  std::cout << in << std::endl;

  auto k = Linx::no_init<Kokkos::complex<double>>("kernel", 3, 3).generate_offsets();
  k += Kokkos::complex<double>(0, 1); // Non-null imaginary part

  std::cout << k << std::endl;

  auto out = Linx::Convolution(k)(in);

  std::cout << out << std::endl;

  auto test = Linx::default_init<Kokkos::complex<double>>("test", 5, 5);
  BOOST_TEST((out.domain() == test.domain()));
  Linx::for_each(
      "test",
      k.domain(),
      KOKKOS_LAMBDA(int i, int j) { test(i, j) = k(i, j); });
  test -= out;

  std::cout << test << std::endl;

  BOOST_TEST(test.contains_only(0));
}

BOOST_AUTO_TEST_SUITE_END()
