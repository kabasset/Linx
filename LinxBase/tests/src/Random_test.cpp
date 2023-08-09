// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Base/DataContainer.h"
#include "Linx/Base/Random.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Random_test)

//-----------------------------------------------------------------------------

template <typename T>
void check_uniform(T) {
  MinimalDataContainer<T> sequence(32);
  sequence.generate(UniformNoise<T>(5, 10));
  for (const auto& e : sequence) {
    BOOST_TEST(e >= 5);
    BOOST_TEST(e <= 10);
  }
  sequence.apply(UniformNoise<T>(50, 100));
  for (const auto& e : sequence) {
    BOOST_TEST(e >= 55);
    BOOST_TEST(e <= 110);
  }
}

void check_uniform(bool) {}

template <typename T>
void check_uniform(std::complex<T>) {
  MinimalDataContainer<std::complex<T>> sequence(32);
  sequence.generate(UniformNoise<std::complex<T>>({5, 1}, {10, 2}));
  for (const auto& e : sequence) {
    BOOST_TEST(e.real() >= 5);
    BOOST_TEST(e.imag() >= 1);
    BOOST_TEST(e.real() <= 10);
    BOOST_TEST(e.imag() <= 2);
  }
  sequence.apply(UniformNoise<std::complex<T>>({50, 10}, {100, 20}));
  for (const auto& e : sequence) {
    BOOST_TEST(e.real() >= 55);
    BOOST_TEST(e.imag() >= 11);
    BOOST_TEST(e.real() <= 110);
    BOOST_TEST(e.imag() <= 22);
  }
}

LINX_TEST_CASE_TEMPLATE(uniform_test) {
  check_uniform(T {});
}

template <typename T>
void check_gaussian(T) {
  MinimalDataContainer<T> sequence(32);
  sequence.generate(GaussianNoise<T>(100, 15));
  sequence.apply(GaussianNoise<T>());
  // FIXME
}

void check_gaussian(bool) {}

template <typename T>
void check_gaussian(std::complex<T>) {}

LINX_TEST_CASE_TEMPLATE(gaussian_test) {
  check_gaussian(T {});
}

template <typename T>
void check_poisson(T) {
  MinimalDataContainer<T> sequence(32);
  sequence.generate(PoissonNoise<T>(20));
  sequence.apply(PoissonNoise<T>());
  // FIXME
}

void check_poisson(bool) {}

template <typename T>
void check_poisson(std::complex<T>) {}

LINX_TEST_CASE_TEMPLATE(poisson_test) {
  check_poisson(T {});
}

BOOST_AUTO_TEST_CASE(reproducible_gaussian_test) {
  MinimalDataContainer<int> sequence_a {10, 100, 1000};
  auto sequence_b = sequence_a;
  sequence_b[1] += 1;
  GaussianNoise<int> noise_a(0, 1, 0);
  GaussianNoise<int> noise_b(0, 1, 0);
  sequence_a.apply(noise_a);
  sequence_b.apply(noise_b);
  BOOST_TEST(sequence_a[0] == sequence_b[0]);
  BOOST_TEST(sequence_a[2] == sequence_b[2]);
}

BOOST_AUTO_TEST_CASE(reproducible_poisson_test) {
  MinimalDataContainer<int> sequence_a {10, 100, 1000};
  auto sequence_b = sequence_a;
  sequence_b[1] += 1;
  StablePoissonNoise<int> noise_a(0, 0);
  StablePoissonNoise<int> noise_b(0, 0);
  sequence_a.apply(noise_a);
  sequence_b.apply(noise_b);
  BOOST_TEST(sequence_a[0] == sequence_b[0]);
  BOOST_TEST(sequence_a[2] == sequence_b[2]);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
