// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Raster/Random.h"
#include "Raster/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Cnes;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Random_test)

//-----------------------------------------------------------------------------

template <typename T>
void checkUniform(T) {
  AlignedRaster<T> raster({3, 2});
  raster.generate(UniformNoise<T>(5, 10));
  for (const auto& e : raster) {
    BOOST_TEST(e >= 5);
    BOOST_TEST(e <= 10);
  }
  raster.apply(UniformNoise<T>(50, 100));
  for (const auto& e : raster) {
    BOOST_TEST(e >= 55);
    BOOST_TEST(e <= 110);
  }
}

void checkUniform(bool) {}

template <typename T>
void checkUniform(std::complex<T>) {
  AlignedRaster<std::complex<T>> raster({3, 2});
  raster.generate(UniformNoise<std::complex<T>>({5, 1}, {10, 2}));
  for (const auto& e : raster) {
    BOOST_TEST(e.real() >= 5);
    BOOST_TEST(e.imag() >= 1);
    BOOST_TEST(e.real() <= 10);
    BOOST_TEST(e.imag() <= 2);
  }
  raster.apply(UniformNoise<std::complex<T>>({50, 10}, {100, 20}));
  for (const auto& e : raster) {
    BOOST_TEST(e.real() >= 55);
    BOOST_TEST(e.imag() >= 11);
    BOOST_TEST(e.real() <= 110);
    BOOST_TEST(e.imag() <= 22);
  }
}

CNES_RASTER_TEST_CASE_TEMPLATE(uniform_test) {
  checkUniform(T {});
}

template <typename T>
void checkGaussian(T) {
  AlignedRaster<T> raster({3, 2});
  raster.generate(GaussianNoise<T>(100, 15));
  raster.apply(GaussianNoise<T>());
  // FIXME
}

void checkGaussian(bool) {}

template <typename T>
void checkGaussian(std::complex<T>) {}

CNES_RASTER_TEST_CASE_TEMPLATE(gaussian_test) {
  checkGaussian(T {});
}

template <typename T>
void checkPoisson(T) {
  AlignedRaster<T> raster({3, 2});
  raster.generate(PoissonNoise<T>(20));
  raster.apply(PoissonNoise<T>());
  // FIXME
}

void checkPoisson(bool) {}

template <typename T>
void checkPoisson(std::complex<T>) {}

CNES_RASTER_TEST_CASE_TEMPLATE(poisson_test) {
  checkPoisson(T {});
}

BOOST_AUTO_TEST_CASE(reproducible_gaussian_test) {
  Raster<int, 1> rasterA({3}, {10, 100, 1000});
  auto rasterB = rasterA;
  rasterB[1] = 1;
  GaussianNoise<int> noiseA(0, 1, 0);
  GaussianNoise<int> noiseB(0, 1, 0);
  BOOST_TEST(rasterA[0] == rasterB[0]);
  BOOST_TEST(rasterA[2] == rasterB[2]);
}

BOOST_AUTO_TEST_CASE(reproducible_poisson_test) {
  Raster<int, 1> rasterA({3}, {10, 100, 1000});
  auto rasterB = rasterA;
  rasterB[1] = 1;
  StablePoissonNoise<int> noiseA(0, 0);
  StablePoissonNoise<int> noiseB(0, 0);
  rasterA.apply(noiseA);
  rasterB.apply(noiseB);
  BOOST_TEST(rasterA[0] == rasterB[0]);
  BOOST_TEST(rasterA[2] == rasterB[2]);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
