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
  BOOST_TEST(image.domain_is_shape_flag == domain.start().static_empty_flag);
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

LINX_QUICK_TEST_CASE_TEMPLATE(wrapper_test)
{
  T v[6] = {1, 1, 1, 1, 1, 1};
  // Linx::Position<3> shape {1, 2, 3};
  // check_ctor(Linx::Raster<T, 3>(Linx::Wrap(v), 1, 2, 3), "", {1, 2, 3}); // FIXME CTAD
  // check_ctor(Linx::Raster<T, 3>(Linx::Wrap(v), shape), "", shape); // FIXME CTAD
}

BOOST_AUTO_TEST_SUITE_END()
