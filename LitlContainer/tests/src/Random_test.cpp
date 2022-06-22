// Copyright (C) 2022, Antoine Basset
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlContainer/Random.h"
#include "LitlContainer/Sequence.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Random_test)

//-----------------------------------------------------------------------------

template <typename T>
void checkUniform(T) {
  Sequence<T> sequence(32);
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

void checkUniform(bool) {}

template <typename T>
void checkUniform(std::complex<T>) {
  Sequence<std::complex<T>> sequence(32);
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

LITL_TEST_CASE_TEMPLATE(uniform_test) {
  checkUniform(T {});
}

template <typename T>
void checkGaussian(T) {
  Sequence<T> sequence(32);
  sequence.generate(GaussianNoise<T>(100, 15));
  sequence.apply(GaussianNoise<T>());
  // FIXME
}

void checkGaussian(bool) {}

template <typename T>
void checkGaussian(std::complex<T>) {}

LITL_TEST_CASE_TEMPLATE(gaussian_test) {
  checkGaussian(T {});
}

template <typename T>
void checkPoisson(T) {
  Sequence<T> sequence(32);
  sequence.generate(PoissonNoise<T>(20));
  sequence.apply(PoissonNoise<T>());
  // FIXME
}

void checkPoisson(bool) {}

template <typename T>
void checkPoisson(std::complex<T>) {}

LITL_TEST_CASE_TEMPLATE(poisson_test) {
  checkPoisson(T {});
}

BOOST_AUTO_TEST_CASE(reproducible_gaussian_test) {
  Sequence<int> sequenceA {10, 100, 1000};
  auto sequenceB = sequenceA;
  sequenceB[1] += 1;
  GaussianNoise<int> noiseA(0, 1, 0);
  GaussianNoise<int> noiseB(0, 1, 0);
  sequenceA.apply(noiseA);
  sequenceB.apply(noiseB);
  BOOST_TEST(sequenceA[0] == sequenceB[0]);
  BOOST_TEST(sequenceA[2] == sequenceB[2]);
}

BOOST_AUTO_TEST_CASE(reproducible_poisson_test) {
  Sequence<int> sequenceA {10, 100, 1000};
  auto sequenceB = sequenceA;
  sequenceB[1] += 1;
  StablePoissonNoise<int> noiseA(0, 0);
  StablePoissonNoise<int> noiseB(0, 0);
  sequenceA.apply(noiseA);
  sequenceB.apply(noiseB);
  BOOST_TEST(sequenceA[0] == sequenceB[0]);
  BOOST_TEST(sequenceA[2] == sequenceB[2]);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
