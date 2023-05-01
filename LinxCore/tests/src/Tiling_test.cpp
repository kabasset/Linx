// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LinxCore/Tiling.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Tiling_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(raster_sections_thickness_test) {
  auto raster = Raster<Index, 3>({8, 3, 4}).fill(-1);
  Index i = 0;
  for (auto& part : sections(raster, 3)) {
    for (auto& e : part) {
      e = i;
    }
    ++i;
  }
  for (const auto& p : raster.domain()) {
    const auto expected = p[2] < 3 ? 0 : 1;
    BOOST_TEST(raster[p] == expected);
  }
}

BOOST_AUTO_TEST_CASE(raster_profiles_ordering_test) {
  const auto raster = Raster<Index, 3>({3, 4, 5}).range();
  const auto parts = profiles<0>(raster);
  const auto plane = parts.domain();
  BOOST_TEST(plane == Box<3>({0, 0, 0}, {0, 3, 4}));
  Index i = 0;
  for (const auto& p : parts) {
    for (const auto& j : p) {
      BOOST_TEST(j == i);
      ++i;
    }
  }
}

BOOST_AUTO_TEST_CASE(raster_rowss_ordering_test) {
  const auto raster = Raster<Index, 3>({3, 4, 5}).range();
  const auto parts = rows(raster);
  const auto plane = parts.domain();
  BOOST_TEST(plane == Box<3>({0, 0, 0}, {0, 3, 4}));
  Index i = 0;
  for (const auto& p : parts) {
    for (const auto& j : p) {
      BOOST_TEST(j == i);
      ++i;
    }
  }
}

BOOST_AUTO_TEST_CASE(raster_profiles_setting_test) {
  auto raster = Raster<Index, 2>({3, 4}).fill(-1);
  auto parts = profiles<1>(raster);
  Index i = 0;
  for (auto& p : parts) {
    for (auto& j : p) {
      j = i;
      ++i;
    }
  }
  for (const auto& p : raster.domain()) {
    BOOST_TEST(raster[p] == p[1] + raster.length(1) * p[0]);
  }
}

BOOST_AUTO_TEST_CASE(raster_tiles_ordering_test) {
  const auto raster = Raster<Index, 3>({3, 4, 5}).range();
  auto parts = tiles(raster, Position<3>({1, 1, 1}));
  Index i = 0;
  for (const auto& p : parts) {
    std::cout << i << std::endl;
    BOOST_TEST(p.size() == 1);
    for (const auto& j : p) {
      BOOST_TEST(j == i);
      ++i;
    }
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
