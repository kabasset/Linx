// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE TilingTest

#include "Linx/Data/Image.h"
#include "Linx/Data/Tiling.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(rows_test)
{
  auto image = Linx::generate("image", KOKKOS_LAMBDA(int i, int, int) { return i; }, 16, 9, 4);
  auto sum = std::array<int, 16> {};
  for (const auto& row : Linx::rows(image)) {
    BOOST_TEST(row.size() == image.extent(0));
    BOOST_TEST(row.size() == sum.size());
    // For testing purposes, better use on_host(image) to avoid copies
    const auto& row_h = Linx::on_host(row);
    for (std::size_t i = 0; i < row.size(); ++i) {
      // FIXME sum[i] += row_h.local(i);
    }
  }
  for (std::size_t i = 0; i < sum.size(); ++i) {
    BOOST_TEST(sum[i] == 9 * 4 * i);
  }
}

BOOST_AUTO_TEST_CASE(profiles_test)
{
  auto image = Linx::generate("image", KOKKOS_LAMBDA(int, int j, int) { return j; }, 16, 9, 4);
  auto sum = std::array<int, 9> {};
  for (const auto& column : Linx::profiles<1>(image)) {
    BOOST_TEST(column.size() == image.extent(1));
    BOOST_TEST(column.size() == sum.size());
    // For testing purposes, better use on_host(image) to avoid copies
    const auto& column_h = Linx::on_host(column);
    for (std::size_t i = 0; i < column.size(); ++i) {
      // FIXME sum[i] += column_h.local(i);
    }
  }
  for (std::size_t i = 0; i < sum.size(); ++i) {
    BOOST_TEST(sum[i] == 16 * 4 * i);
  }
}

BOOST_AUTO_TEST_SUITE_END()
