// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE PipelineTest

#include "Linx/Base/Functional.h"
#include "Linx/Base/Random.h"
#include "Linx/Data/Image.h"
#include "Linx/Run/Logging.h"
#include "Linx/Run/PipelineTasks.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

namespace P = Linx::Pipeline;

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(sequence_api_test)
{
  auto size = 10;
  auto logger = Linx::TimerLogger();
  auto [out] = P::Run("(1 + 2) * 3", logger) // init
      | Linx::Constant(1) | Linx::Slice(0, size) // input
      | P::Apply(Linx::Add(2), Linx::Multiply(3)); // pixelwise operations
  logger("Done");
  BOOST_TEST((out.ssize() == size));
  BOOST_TEST(out.contains_only(9));
}

BOOST_AUTO_TEST_CASE(image_api_test)
{
  auto width = 10;
  auto height = 3;
  auto logger = Linx::TimerLogger();
  auto [out] = P::Run("(1 + 2) * 3", logger) // init
      | Linx::Constant(1) | Linx::Box({0, 0}, {width, height}) // input
      | P::Apply(Linx::Add(2), Linx::Multiply(3)); // pixelwise operations
  logger("Done");
  BOOST_TEST((out.shape() == Linx::Position({width, height})));
  BOOST_TEST(out.contains_only(9));
}

BOOST_AUTO_TEST_CASE(diadic_test)
{
  auto size = 100;
  auto logger = Linx::TimerLogger();
  auto [out] = P::Run("1 + 2", logger) // init
      | P::Input(Linx::Constant(1), Linx::Constant(2)) | Linx::Slice(0, size) // inputs
      | P::Apply(Linx::Add()); // merge
  logger("Done");
  BOOST_TEST(out.contains_only(3));
}

BOOST_AUTO_TEST_SUITE_END()
