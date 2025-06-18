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
  auto static_size_sequence = Linx::Sequence<int, 3>();
  ASSERT(static_size_sequence.n == 3);
  ASSERT(static_size_sequence.size() == 3);
  ASSERT(static_size_sequence.rank() == 1);

  auto dynamic_size_sequence = Linx::Sequence<int, -1>(3);
  ASSERT(dynamic_size_sequence.n == -1);
  ASSERT(dynamic_size_sequence.size() == 3);
  ASSERT(dynamic_size_sequence.rank() == 1);
  //! [sequence_size]
}

BOOST_AUTO_TEST_CASE(sequence_null_size_test)
{
  //! [sequence_null_size]
  auto static_size_sequence = Linx::Sequence<int, 0>();
  ASSERT(static_size_sequence.n == 0);
  ASSERT(static_size_sequence.size() == 0);
  ASSERT(static_size_sequence.rank() == 1);

  auto dynamic_size_sequence = Linx::Sequence<int, -1>(0);
  ASSERT(dynamic_size_sequence.n == -1);
  ASSERT(dynamic_size_sequence.size() == 0);
  ASSERT(dynamic_size_sequence.rank() == 1);
  //! [sequence_null_size]
}

BOOST_AUTO_TEST_CASE(image_rank_test)
{
  //! [image_rank]
  auto static_rank_image = Linx::Image<int, 3>(10, 20, 30);
  ASSERT(static_rank_image.n == 3);
  ASSERT(static_rank_image.rank() == 3);
  ASSERT(static_rank_image.shape() == Linx::Position({10, 20, 30}));
  ASSERT(static_rank_image.size() == 10 * 20 * 30);
  ASSERT(static_rank_image.extent(0) == 10);
  ASSERT(static_rank_image.extent(1) == 20);
  ASSERT(static_rank_image.extent(2) == 30);

  auto dynamic_rank_image = Linx::Image<int, -1>(10, 20, 30);
  ASSERT(dynamic_rank_image.n == -1);
  ASSERT(dynamic_rank_image.rank() == 3);
  ASSERT(static_rank_image.size() == 10 * 20 * 30);
  ASSERT(static_rank_image.shape() == Linx::Position<-1>({10, 20, 30}));
  ASSERT(static_rank_image.extent(0) == 10);
  ASSERT(static_rank_image.extent(1) == 20);
  ASSERT(static_rank_image.extent(2) == 30);
  //! [image_rank]
}

BOOST_AUTO_TEST_CASE(image_null_rank_test)
{
  //! [image_null_rank]
  auto null_image = Linx::Image<int, 0>();
  ASSERT(null_image.n == 0);
  ASSERT(null_image.rank() == 0);
  ASSERT(null_image.size() == 0);
  ASSERT(null_image.empty());
  ASSERT(null_image.shape() == Linx::Position<0>({}));
  ASSERT(null_image.data() == nullptr);
  //! [image_null_rank]
}

BOOST_AUTO_TEST_CASE(access_test)
{
  //! [access]
  auto image = Linx::Raster<int, 2>(4, 3);
  for (auto e : image) {
    ASSERT(e == 0);
  }

  image(0, 1) = 1;
  ASSERT(image(0, 1) == 1);

  auto p = Linx::Position<2>({0, 1}); // FIXME NVCC bug: removing <2> makes p a Sequence on device!
  image[p] = 2;
  ASSERT(image[p] == 2);
  ASSERT(image[{0, 1}] == 2);
  //! [access]
}

BOOST_AUTO_TEST_SUITE_END()
