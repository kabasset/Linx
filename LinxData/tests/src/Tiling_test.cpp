// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Data/Tiling.h"

#include <boost/test/unit_test.hpp>
#include <omp.h>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Tiling_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(raster_multisections_thickness_test)
{
  auto raster = Raster<Index, 3>({8, 3, 4}).fill(-1);
  Index i = 0;
  for (auto part : sections(raster, 3)) {
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

BOOST_AUTO_TEST_CASE(raster_sections_test)
{
  auto raster = Raster<Index, 3>({8, 3, 4}).fill(-1);
  Index i = 0;
  for (auto part : sections(raster)) {
    for (auto& e : part) {
      e = i;
    }
    ++i;
  }
  for (const auto& p : raster.domain()) {
    const auto expected = p[2];
    BOOST_TEST(raster[p] == expected);
  }
}

BOOST_AUTO_TEST_CASE(raster_profiles_ordering_test)
{
  const auto raster = Raster<Index, 3>({3, 4, 5}).range();
  const auto parts = profiles<0>(raster);
  const auto plane = rasterize(parts).domain(); // FIXME implement parts.domain(), parts.shape()
  BOOST_TEST(plane == Box<2>({0, 0}, {3, 4}));
  Index i = 0;
  for (const auto& p : parts) {
    for (const auto& j : p) {
      BOOST_TEST(j == i);
      ++i;
    }
  }
}

BOOST_AUTO_TEST_CASE(raster_rows_ordering_test)
{
  const auto raster = Raster<Index, 3>({3, 4, 5}).range();
  auto parts = rows(raster);
  const auto parts_raster = rasterize(parts);
  const auto plane = parts_raster.domain();
  BOOST_TEST(plane == Box<2>({0, 0}, {3, 4}));
  Index i = 0;
  for (const auto& p : parts) {
    for (const auto& j : p) {
      BOOST_TEST(j == i);
      // FIXME test parts_raster
      ++i;
    }
  }
}

BOOST_AUTO_TEST_CASE(raster_profiles_setting_test)
{
  auto raster = Raster<Index, 2>({3, 4}).fill(-1);
  Index i = 0;
  for (auto p : profiles<1>(raster)) {
    for (auto& j : p) {
      j = i;
      ++i;
    }
  }
  for (const auto& p : raster.domain()) {
    BOOST_TEST(raster[p] == p[1] + raster.length(1) * p[0]);
  }
}

BOOST_AUTO_TEST_CASE(raster_tiles_ordering_test)
{
  const auto raster = Raster<Index, 3>({3, 4, 5}).range();
  auto parts = tiles(raster, Position<3>({1, 1, 1}));
  const auto parts_raster = rasterize(parts);
  Index i = 0;
  for (const auto& p : parts) {
    BOOST_TEST(p.size() == 1);
    BOOST_TEST(parts_raster[i] == p);
    for (const auto& j : p) {
      BOOST_TEST(j == i);
      ++i;
    }
  }
}

BOOST_AUTO_TEST_CASE(raster_tiles_parallelization_test)
{
  auto raster = Raster<Index, 2>({4, 3});
  auto generator = tiles(raster, Position<2>({1, 1}));
  auto parts = rasterize(generator);
  std::vector<Index> threads(raster.size());

#pragma omp parallel for
  for (std::size_t i = 0; i < parts.size(); ++i) {
    const auto n = omp_get_thread_num();
    threads[i] += n;
    parts[i] += n;
  }

  BOOST_TEST(raster.container() == threads);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
