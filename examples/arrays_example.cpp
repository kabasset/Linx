// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ArraysExample

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

#define ASSERT(...) BOOST_TEST((__VA_ARGS__))

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(sequence_size_test)
{
  //! [sequence_size]
  auto a = Linx::Sequence<int, 3>();
  ASSERT(a.n == 3);
  ASSERT(a.size() == 3);
  ASSERT(a.rank() == 1);

  auto b = Linx::Sequence<int, -1>(3);
  ASSERT(b.n == -1);
  ASSERT(b.size() == 3);
  ASSERT(b.rank() == 1);
  //! [sequence_size]
}

BOOST_AUTO_TEST_CASE(sequence_null_size_test)
{
  //! [sequence_null_size]
  auto a = Linx::Sequence<int, 0>();
  ASSERT(a.n == 0);
  ASSERT(a.size() == 0);
  ASSERT(a.rank() == 1);

  auto b = Linx::Sequence<int, -1>(0);
  ASSERT(b.n == -1);
  ASSERT(b.size() == 0);
  ASSERT(b.rank() == 1);
  //! [sequence_null_size]
}

BOOST_AUTO_TEST_CASE(image_rank_test)
{
  //! [image_rank]
  auto a = Linx::Image<int, 3>(10, 20, 30);
  ASSERT(a.n == 3);
  ASSERT(a.rank() == 3);
  ASSERT(a.size() == 10 * 20 * 30);
  ASSERT(a.shape() == Linx::Position({10, 20, 30}));
  ASSERT(a.extent(0) == 10);
  ASSERT(a.extent(1) == 20);
  ASSERT(a.extent(2) == 30);

  auto b = Linx::Image<int, -1>(10, 20, 30);
  ASSERT(b.n == -1);
  ASSERT(b.rank() == 3);
  ASSERT(b.size() == 10 * 20 * 30);
  ASSERT(b.shape() == Linx::Position<-1>({10, 20, 30}));
  ASSERT(b.extent(0) == 10);
  ASSERT(b.extent(1) == 20);
  ASSERT(b.extent(2) == 30);
  //! [image_rank]
}

BOOST_AUTO_TEST_CASE(image_null_rank_test)
{
  //! [image_null_rank]
  auto a = Linx::Image<int, 0>();
  ASSERT(a.n == 0);
  ASSERT(a.rank() == 0);
  ASSERT(a.size() == 0);
  ASSERT(a.empty());
  ASSERT(a.shape() == Linx::Position<0>());
  ASSERT(a.data() == nullptr);
  //! [image_null_rank]
}

BOOST_AUTO_TEST_CASE(rowwise_test)
{
  //! [rowwise]
  auto a = Linx::rowwise("1D int", {1, 2});
  auto b = Linx::rowwise("2D char", {{'a', 'b'}, {'c', 'd'}});
  auto c = Linx::rowwise<float>("3D float", {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}});
  //! [rowwise]

  static_assert(std::is_same_v<decltype(a)::element_type, int>);
  ASSERT(a.n == 1);
  ASSERT(a.label() == "1D int");
  ASSERT(a.size() == 2);
  const auto& a_on_host = Linx::on_host(a);
  ASSERT(a_on_host(0) == 1);
  ASSERT(a_on_host(1) == 2);

  static_assert(std::is_same_v<decltype(b)::element_type, char>);
  ASSERT(b.n == 2);
  ASSERT(b.label() == "2D char");
  ASSERT(b.size() == 4);
  const auto& b_on_host = Linx::on_host(b);
  for (int j = 0; j < 2; ++j) {
    for (int i = 0; i < 2; ++i) {
      ASSERT(b_on_host(i, j) == char('a' + 2 * j + i));
    }
  }

  static_assert(std::is_same_v<decltype(c)::element_type, float>);
  ASSERT(c.n == 3);
  ASSERT(c.label() == "3D float");
  ASSERT(c.size() == 8);
  const auto& c_on_host = Linx::on_host(c);
  for (int k = 0; k < 2; ++k) {
    for (int j = 0; j < 2; ++j) {
      for (int i = 0; i < 2; ++i) {
        ASSERT(c_on_host(i, j, k) == float(1 + 4 * k + 2 * j + i));
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(access_test)
{
  //! [access]
  auto a = Linx::Raster<int, 2>(4, 3);
  for (auto a_i : a) {
    ASSERT(a_i == 0);
  }

  a(0, 1) = 1;
  ASSERT(a(0, 1) == 1);

  auto p = Linx::Position<2>({0, 1}); // FIXME NVCC bug: removing <2> makes p a Sequence on device!
  a[p] = 2;
  ASSERT(a[p] == 2);
  ASSERT(a[{0, 1}] == 2);
  //! [access]
}

BOOST_AUTO_TEST_SUITE_END()
