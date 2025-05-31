// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE LinearInterpolationTest

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/Resampling.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(sequence_test)
{
  const auto a = Linx::Position({-1, 0, 1});
  const auto interpolated = Linx::Interpolation(a, Linx::Linear());
  const auto& interpolated_h = Linx::on_host(interpolated);
  BOOST_TEST(interpolated_h(0.0) == -1);
  BOOST_TEST(interpolated_h(0.5) == -0.5);
  BOOST_TEST(interpolated_h(1.0) == 0);
  BOOST_TEST(interpolated_h(1.25) == 0.25);
  BOOST_TEST(interpolated_h(2.0) == 1);
}

BOOST_AUTO_TEST_CASE(image_2d_test)
{
  const auto a = Linx::Raster<float>(2, 3);
  a(0, 0) = -1;
  a(1, 0) = -10;
  a(0, 1) = 0;
  a(1, 1) = 0;
  a(0, 2) = 1;
  a(1, 2) = 10;
  const auto interpolated = Linx::Interpolation(a, Linx::Linear());
  const auto& interpolated_h = Linx::on_host(interpolated);
  BOOST_TEST(interpolated_h(0.0, 0.0) == -1);
  BOOST_TEST(interpolated_h(0.5, 0.5) == -2.75);
  BOOST_TEST(interpolated_h(0.5, 1.0) == 0);
  BOOST_TEST(interpolated_h(0.5, 1.25) == 1.375);
  BOOST_TEST(interpolated_h(1.0, 2.0) == 10);
}

BOOST_AUTO_TEST_SUITE_END()
