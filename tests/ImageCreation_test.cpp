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
    const Linx::Specialization<Linx::Box> auto& domain,
    auto value)
{
  std::cout << image << std::endl;

  BOOST_TEST(image.n == domain.n);
  if (image.n == 1) {
    BOOST_TEST(image.static_contiguous_flag);
  }
  BOOST_TEST(image.static_domain_flag == domain.static_flag);
  BOOST_TEST(image.static_start_at_origin_flag == domain.static_start_at_origin_flag);
  BOOST_TEST(image.label() == label);
  BOOST_TEST(image.rank() == domain.rank());
  BOOST_TEST(image.domain() == domain);
  for (int i = 0; i < image.rank(); ++i) {
    BOOST_TEST(image.extent(i) == domain.extent(i));
  }

  const auto size = domain.size();
  BOOST_TEST(image.size() == size);
  BOOST_TEST(image.ssize() == size);
  if (size == 0) {
    BOOST_TEST(image.empty());
  } else {
    BOOST_TEST(not image.empty());
    BOOST_TEST(image.data());
    BOOST_TEST((image.cdata() == image.data()));
    BOOST_TEST(image.contains_only(value));
  }
}

LINX_TEST_CASE_TEMPLATE(default_init_test)
{
  check_ctor(Linx::default_init<T>("extents", 4, 3), "extents", Linx::shape(4, 3), T {});
  check_ctor(Linx::default_init<T>("shape", Linx::shape<4, 3>()), "shape", Linx::shape<4, 3>(), T {});
  check_ctor(Linx::default_init<T>("box", Linx::Box({-2, -1}, {2, 2})), "box", Linx::Box({-2, -1}, {2, 2}), T {});
}

BOOST_AUTO_TEST_CASE(rowwise_test)
{
  check_ctor(Linx::rowwise("1D", {1, 1, 1}), "1D", Linx::shape<3>(), 1);
  check_ctor(Linx::rowwise("2D", {{1, 1}, {1, 1}}), "2D", Linx::shape<2, 2>(), 1);
  check_ctor(Linx::rowwise("3D", {{{1, 1, 1}}}), "3D", Linx::shape<3, 1, 1>(), 1);
}

BOOST_AUTO_TEST_CASE(wrapper_test)
{
  int v[6] = {1, 1, 1, 1, 1, 1};

  // Static rank
  check_ctor(Linx::wrap(v, 1, 2, 3), "", Linx::shape(1, 2, 3), 1);

  // Static domain
  const auto shape = Linx::shape<1, 2, 3>();
  check_ctor(Linx::wrap(v, shape), "", shape, 1);

  // Offset domain
  using namespace Linx::Literals;
  const auto box = Linx::Box(Linx::vec(-1, -2, -3), Linx::vec<3_D, 0>());
  check_ctor(Linx::wrap(v, box), "", box, 1);
}

LINX_TEST_CASE_TEMPLATE(fill_test)
{
  check_ctor(Linx::fill("extents", 1, 4, 3), "extents", Linx::shape(4, 3), 1);
  check_ctor(Linx::fill("shape", 1, Linx::shape<4, 3>()), "shape", Linx::shape<4, 3>(), 1);
  check_ctor(Linx::fill("box", 1, Linx::Box({-2, -1}, {2, 2})), "box", Linx::Box({-2, -1}, {2, 2}), 1);
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
