// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ImageCreationTest

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

void check_ctor(
    const Linx::Specialization<Linx::Image> auto& image,
    const std::string& label,
    const Linx::Specialization<Linx::Box> auto& domain)
{
  BOOST_TEST(image.n == domain.n);
  BOOST_TEST(image.static_domain_flag == domain.static_flag);
  BOOST_TEST(image.static_start_at_origin_flag == domain.static_start_at_origin_flag);
  BOOST_TEST(image.rank() == domain.rank());
  BOOST_TEST(image.label() == label);

  const auto size = domain.size();
  BOOST_TEST(image.size() == size);
  BOOST_TEST(image.ssize() == size);
  if (size == 0) {
    BOOST_TEST(image.empty());
    // FIXME for Raster: BOOST_TEST((end(image) == begin(image)));
  } else {
    BOOST_TEST(not image.empty());
    BOOST_TEST(image.data());
    BOOST_TEST((image.cdata() == image.data()));
    // FIXME for Raster: BOOST_TEST((end(image) != begin(image)));
    BOOST_TEST(image.contains_only(1));
  }
}

LINX_TEST_CASE_TEMPLATE(rowwise_test)
{
  check_ctor(Linx::rowwise("1D", {1, 1, 1}), "1D", Linx::shape<3>());
  check_ctor(Linx::rowwise("2D", {{1, 1}, {1, 1}}), "2D", Linx::shape<2, 2>());
  check_ctor(Linx::rowwise("3D", {{{1, 1, 1}}}), "3D", Linx::shape<3, 1, 1>());
}

BOOST_AUTO_TEST_CASE(wrapper_test)
{
  int v[6] = {1, 1, 1, 1, 1, 1};

  // Static rank
  check_ctor(Linx::wrap(v, 1, 2, 3), "", Linx::shape(1, 2, 3));

  // Static domain
  const auto shape = Linx::shape<1, 2, 3>();
  check_ctor(Linx::wrap(v, shape), "", shape);

  // Offset domain
  using namespace Linx::Literals;
  const auto box = Linx::Box(Linx::vec(-1, -2, -3), Linx::vec<3_D, 0>());
  check_ctor(Linx::wrap(v, box), "", box);
}

BOOST_AUTO_TEST_CASE(generate_test)
{
  auto domain = Linx::Box(Linx::vec({-1, -2}), Linx::vec<3, 4>());
  auto a = Linx::generate<Kokkos::HostSpace>("a", KOKKOS_LAMBDA(int i, int j) { return i + j * 4; }, domain);
  BOOST_TEST(a.domain() == domain);

  for (int j = -2; j < 4; ++j) {
    for (int i = -1; i < 3; ++i) {
      BOOST_TEST(a(i, j) == i + j * 4);
    }
  }

  BOOST_TEST(a.front() == -1 - 2 * 4);
  BOOST_TEST(a.origin() == 0);
  BOOST_TEST(a.back() == 2 + 3 * 4);
}

BOOST_AUTO_TEST_SUITE_END()
