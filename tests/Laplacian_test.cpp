// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE LaplacianTest

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/LinearFiltering.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(box_test)
{
  const auto filter = Linx::separable_laplacian<0, 1>();
  const auto box = Linx::box(filter.footprint());
  BOOST_TEST((box == Linx::Box<2>({-1, -1}, {2, 2})));
}

BOOST_AUTO_TEST_SUITE_END()
