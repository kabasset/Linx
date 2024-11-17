// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ImageLinearFilteringTest

#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/LinearFiltering.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(correlation_impulse_response_test)
{
  auto in = Linx::Image<int, 2>("impulse", 5, 5);
  const auto& in_on_host = Linx::on_host(in);
  in_on_host(2, 2) = 1;
  Kokkos::deep_copy(in_on_host.container(), in.container());

  auto k = Linx::Image<Kokkos::complex<double>, 2>("kernel", 3, 3).fill_with_offsets(); // Unique values
  k += Kokkos::complex<double>(0, 1); // Non-null imaginary part
  auto out = Linx::Correlation(k)(in);

  auto test = Linx::Image<Kokkos::complex<double>, 2>("test", 5, 5);
  BOOST_TEST((out.domain() == test.domain()));
  Linx::for_each(
      "test",
      k.domain(),
      KOKKOS_LAMBDA(int i, int j) { test(2 - i, 2 - j) = Kokkos::conj(k(i, j)); });
  test -= out;

  BOOST_TEST(test.contains_only(0));
}

BOOST_AUTO_TEST_CASE(correlation_unit_response_test)
{
  auto in = Linx::Image<int, 2>("unit", 5, 5).fill(1);

  auto k = Linx::Image<int, 2>("kernel", 3, 3).fill_with_offsets();
  auto out = Linx::Correlation(k)(in);

  auto test = Linx::Image<int, 2>("test", 5, 5);
  BOOST_TEST((out.domain() == test.domain()));
  auto sum = Linx::sum(k);
  Linx::for_each(
      "test",
      k.domain(),
      KOKKOS_LAMBDA(int i, int j) { test(2 - i, 2 - j) = sum; });
  test -= out;

  BOOST_TEST(test.contains_only(0));
}

BOOST_AUTO_TEST_CASE(convolution_impulse_response_test)
{
  auto in = Linx::Image<int, 2>("impulse", 5, 5);
  const auto& in_on_host = Linx::on_host(in);
  in_on_host(2, 2) = 1;
  Kokkos::deep_copy(in_on_host.container(), in.container());

  auto k = Linx::Image<Kokkos::complex<double>, 2>("kernel", 3, 3).fill_with_offsets(); // Unique values
  k += Kokkos::complex<double>(0, 1); // Non-null imaginary part
  auto out = Linx::Convolution(k)(in);

  auto test = Linx::Image<Kokkos::complex<double>, 2>("test", 5, 5);
  BOOST_TEST((out.domain() == test.domain()));
  Linx::for_each(
      "test",
      k.domain(),
      KOKKOS_LAMBDA(int i, int j) { test(i, j) = k(i, j); });
  test -= out;

  BOOST_TEST(test.contains_only(0));
}

BOOST_AUTO_TEST_SUITE_END()
