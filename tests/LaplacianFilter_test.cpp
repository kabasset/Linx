// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE LaplacianFilterTest

#include "Linx/Data/Box.h"
#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/LinearFiltering.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(box_test)
{
  const auto filter = Linx::separable_laplacian<0, 1>();
  const auto box = Linx::bbox(filter.footprint());
  BOOST_TEST((box == Linx::Box({-1, -1}, {2, 2})));
}

BOOST_AUTO_TEST_CASE(impulse_test)
{
  using namespace Linx::Literals;

  auto domain = Linx::cube<3_D, 2>();
  const auto in_h = Linx::Raster<int, decltype(domain)>("in", domain);
  in_h.origin() = 1;
  const auto out = Linx::separable_laplacian<0, 1, 2>(-1)(Linx::on_device(in_h));
  static_assert(std::is_same_v<decltype(out)::execution_space, Kokkos::DefaultExecutionSpace>);
  const auto expected_h = Linx::Raster<int, decltype(domain)>("exp", domain);
  expected_h(0, 0, -1) = -1;
  expected_h(0, -1, 0) = -1;
  expected_h(-1, 0, 0) = -1;
  expected_h(0, 0, 0) = 6;
  expected_h(1, 0, 0) = -1;
  expected_h(0, 1, 0) = -1;
  expected_h(0, 0, 1) = -1;
  auto expected = Linx::on_device(expected_h);
  BOOST_TEST((out == expected));
}

BOOST_AUTO_TEST_SUITE_END()
