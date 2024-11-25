// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE AlgorithmMedian

#include "Linx/Base/SelectNet.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>
#include <ranges>

template <typename T>
using Parity = typename T::Parity;

template <int N, typename TParity = void>
void check_method()
{
  using Net = Linx::SelectNet<N>;
  if constexpr (std::is_same_v<TParity, void>) {
    BOOST_TEST((not Kokkos::is_detected_v<Parity, Net>));
  } else {
    BOOST_TEST((std::is_same_v<typename Net::Parity, TParity>));
  }
}

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(method_test)
{
  check_method<-2, Linx::EvenNumber>();
  check_method<-1, Linx::OddNumber>();
  check_method<0, Linx::Forward>();
  check_method<1>();
  check_method<2>();
  check_method<3>();
  check_method<4>();
  check_method<5>();
  check_method<6>();
  check_method<7>();
  check_method<8>();
  check_method<9>();
  check_method<25>();
  check_method<27>();
  check_method<666, Linx::EvenNumber>();
  check_method<999, Linx::OddNumber>();
}

BOOST_AUTO_TEST_CASE(odd_increasing_test)
{
  const Linx::GPosition a({0, 1, 10, 100, 1000});
  BOOST_TEST(Linx::median<Linx::SelectNet<5>>(a) == 10);
}

BOOST_AUTO_TEST_CASE(odd_decreasing_test)
{
  const Linx::GPosition a({1000, 100, 10, 1, 0});
  BOOST_TEST(Linx::median<Linx::SelectNet<5>>(a) == 10);
}

BOOST_AUTO_TEST_CASE(even_random_test)
{
  const Linx::GPosition a({1, 100, 0, 10, 10000, 1000});
  BOOST_TEST(Linx::median<Linx::SelectNet<6>>(a) == 550);
}

BOOST_AUTO_TEST_SUITE_END()
