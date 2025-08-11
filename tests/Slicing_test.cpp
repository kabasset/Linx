// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE SlicingTest

#include "Linx/Base/Slice.h"
#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>
#include <sstream>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(span_test)
{
  int start = 3;
  int stop = 14;
  auto span = Linx::Slice(start, stop);
  BOOST_TEST(span.start() == start);
  BOOST_TEST(span.stop() == stop);
  BOOST_TEST(kokkos_slice(span).first == start);
  BOOST_TEST(kokkos_slice(span).second == stop);

  auto str = (std::stringstream() << span).str();
  BOOST_TEST(str == std::to_string(start) + ':' + std::to_string(stop));
}

BOOST_AUTO_TEST_CASE(span_unbounded_singleton_slice_test)
{
  auto image = Linx::no_init<float>("image", 16, 9, 4);
  Linx::for_each(
      "init",
      image.domain(),
      KOKKOS_LAMBDA(int i, int j, int k) { image(i, j, k) = i + j + k; });
  auto slice = image[Linx::Slice(1, 5)()(3)];
  BOOST_TEST(slice.label() == image.label());
  BOOST_TEST(slice.n == 2);
  BOOST_TEST(slice.static_start_at_origin_flag);
  BOOST_TEST(slice.domain() == Linx::Box({4, 9}));

  auto box = Linx::Box({0, 0}, {4, 9});
  auto diff = Linx::no_init<float>("diff", box);
  Linx::for_each(
      "test",
      box,
      KOKKOS_LAMBDA(int i, int j) { diff(i, j) = slice(i, j) - image(i + 1, j, 3); });
  BOOST_TEST(Linx::norm<0>(diff) == 0);
}

BOOST_AUTO_TEST_CASE(box_slice_test)
{
  auto image = Linx::no_init<float>("image", 16, 9, 4);
  auto box = Linx::Box({1, 0, 3}, {5, 9, 4});
  Linx::for_each(
      "init",
      image.domain(),
      KOKKOS_LAMBDA(int i, int j, int k) { image(i, j, k) = i + j + k; });
  auto slice = image[box];
  BOOST_TEST(slice.label() == image.label());
  BOOST_TEST(slice.n == 3);
  BOOST_TEST(slice.static_start_at_origin_flag);
  BOOST_TEST(slice.domain() == Linx::Box({4, 9, 1}));

  auto diff = Linx::no_init<float>("diff", box - box.start());
  Linx::for_each(
      "test",
      diff.domain(),
      KOKKOS_LAMBDA(int i, int j, int k) { diff(i, j, k) = slice(i, j, k) - image(i + 1, j, k + 3); });
  BOOST_TEST(Linx::norm<0>(diff) == 0);
}

BOOST_AUTO_TEST_CASE(index_range_slice_test)
{
  auto image = Linx::no_init<float>("image", 16, 9, 4);
  for_each(
      "init",
      image.domain(),
      KOKKOS_LAMBDA(int i, int j, int k) { image(i, j, k) = i + j + k; });
  auto slice = image[Linx::Slice(1, 3)];
  BOOST_TEST(slice.label() == image.label());
  BOOST_TEST(slice.n == 3);
  BOOST_TEST(slice.static_start_at_origin_flag);
  BOOST_TEST(slice.domain() == Linx::Box({16, 9, 2}));

  auto diff = Linx::no_init<float>("diff", slice.domain());
  Linx::for_each(
      "test",
      slice.domain(),
      KOKKOS_LAMBDA(int i, int j, int k) { diff(i, j, k) = slice(i, j, k) - image(i, j, k + 1); });
  BOOST_TEST(Linx::norm<0>(diff) == 0);
}

BOOST_AUTO_TEST_CASE(index_slice_test)
{
  auto image = Linx::no_init<float>("image", 16, 9, 4);
  for_each(
      "init",
      image.domain(),
      KOKKOS_LAMBDA(int i, int j, int k) { image(i, j, k) = i + j + k; });
  auto slice = image[Linx::Slice(1)];
  BOOST_TEST(slice.label() == image.label());
  BOOST_TEST(slice.n == 2);
  BOOST_TEST(slice.static_start_at_origin_flag);
  BOOST_TEST(slice.domain() == Linx::Box({16, 9}));

  auto diff = Linx::no_init<float>("diff", slice.domain());
  // Linx::for_each("test", slice.domain(), KOKKOS_LAMBDA(int i, int j) { diff(i, j) = slice(i, j) - image(i, j, 1); });
  BOOST_TEST(Linx::norm<0>(diff) == 0);
}

BOOST_AUTO_TEST_SUITE_END()
