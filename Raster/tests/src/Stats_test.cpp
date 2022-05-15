// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Raster/Stats.h"

#include <boost/test/unit_test.hpp>

using namespace Cnes;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Stats_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(robust_test) {
  std::vector<int> in {2, 1, 9, 4, 1, 2, 6};
  Stats<StatsMode::Robust> stats(in);
  BOOST_TEST(stats.min == 1);
  BOOST_TEST(stats.max == 9);
  BOOST_TEST(stats.mean == 2);
  BOOST_TEST(stats.deviation == 1);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
