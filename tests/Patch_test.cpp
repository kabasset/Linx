// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE PatchTest

#include "Linx/Data/Patch.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>
#include <sstream>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(patch_unbounded_singleton_patch_test)
{
  auto image = Linx::no_init<float>("image", 16, 9, 4);
  for_each("init", image.domain(), KOKKOS_LAMBDA(int i, int j, int k) { image(i, j, k) = i + j + k; });
  auto patch = Linx::Patch(image, Linx::Slice(1, 5)()(3));
  BOOST_TEST((Linx::root(patch) == image));
  BOOST_TEST((Linx::root(patch).container() == image.container()));
  BOOST_TEST(patch.n == 3);
  const auto& domain = Linx::bbox(patch.domain());
  BOOST_TEST(domain.extent(0) == 4);
  BOOST_TEST(domain.extent(1) == 9);
  BOOST_TEST(domain.extent(2) == 1);
  BOOST_TEST(domain.start(0) == 1);
  BOOST_TEST(domain.stop(0) == 5);
  BOOST_TEST(domain.start(1) == 0);
  BOOST_TEST(domain.stop(1) == 9);
  BOOST_TEST(domain.start(2) == 3);
  BOOST_TEST(domain.stop(2) == 4);

  auto box = Linx::Box({1, 0}, {5, 9});
  auto diff = Linx::no_init<float>("diff", box - box.start()); // FIXME box.shape());
  auto i0 = box.start(0);
  auto j0 = box.start(1);
  Linx::for_each("test", box, KOKKOS_LAMBDA(int i, int j) { diff(i - i0, j - j0) = patch(i, j, 3) - image(i, j, 3); });
  BOOST_TEST(Linx::norm<0>(diff) == 0);
}

BOOST_AUTO_TEST_CASE(patch_of_patch_test)
{
  auto image = Linx::no_init<int>("image", 10, 8);
  BOOST_TEST(image.domain() == Linx::Box({10, 8}));

  auto box_a = Linx::Box({1, -1}, {11, 7});
  auto patch_a = Linx::Patch(image, box_a);
  BOOST_TEST(patch_a.domain() == Linx::Box({1, 0}, {10, 7}));

  auto box_b = Linx::Box({-1, 1}, {9, 10});
  auto patch_b = Linx::Patch(patch_a, box_b);
  BOOST_TEST(patch_b.domain() == Linx::Box({1, 1}, {9, 7}));
}

BOOST_AUTO_TEST_SUITE_END()
