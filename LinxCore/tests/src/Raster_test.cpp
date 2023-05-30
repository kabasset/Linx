// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LinxCore/Raster.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Raster_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(index_test) {

  /* Fixed dimension */
  Position<4> fixedShape;
  for (auto& length : fixedShape) {
    length = std::rand();
  }
  Position<4> fixedPos;
  for (auto& coord : fixedPos) {
    coord = std::rand();
  }
  auto fixedIndex = Internal::IndexRecursionImpl<4>::index(fixedShape, fixedPos);
  BOOST_TEST(
      fixedIndex ==
      fixedPos[0] + fixedShape[0] * (fixedPos[1] + fixedShape[1] * (fixedPos[2] + fixedShape[2] * (fixedPos[3]))));

  /* Variable dimension */
  Position<-1> variableShape(fixedShape);
  Position<-1> variablePos(fixedPos);
  auto variableIndex = Internal::IndexRecursionImpl<-1>::index(variableShape, variablePos);
  BOOST_TEST(variableIndex == fixedIndex);
}

BOOST_AUTO_TEST_CASE(ptrraster_data_test) {
  int data[] = {0, 1, 2};
  PtrRaster<int, 1> raster({3}, data);
  BOOST_TEST(raster.data() != nullptr);
  BOOST_TEST(raster[{0}] == 0);
}

BOOST_AUTO_TEST_CASE(const_ptrraster_data_test) {
  const int cData[] = {3, 4, 5};
  PtrRaster<const int, 1> cRaster({3}, cData);
  BOOST_TEST(cRaster.data() != nullptr);
  BOOST_TEST(cRaster[{0}] == 3);
}

BOOST_AUTO_TEST_CASE(vecraster_data_test) {
  VecRaster<int, 1> vecRaster({3});
  BOOST_TEST(vecRaster.data() != nullptr);
  BOOST_TEST(vecRaster[{0}] == 0);
}

BOOST_AUTO_TEST_CASE(const_vecraster_data_test) {
  const VecRaster<int, 1> cVecRaster({3});
  BOOST_TEST(cVecRaster.data() != nullptr);
  BOOST_TEST(cVecRaster[{0}] == 0);
}

BOOST_AUTO_TEST_CASE(alignedraster_owned_and_shared_test) {
  constexpr Index width = 3;
  constexpr Index height = 4;
  constexpr Index size = width * height;
  AlignedRaster<double> owner({width, height});
  BOOST_TEST(owner.size() == size);
  const AlignedRaster<double> sharer({width, height}, owner.data());
  BOOST_TEST(not sharer.owns());
  BOOST_TEST(sharer.size() == size);
  BOOST_TEST(sharer.data() == owner.data());
  AlignedRaster<const double> observer({width, height}, owner.data());
  BOOST_TEST(observer.size() == size);
  BOOST_TEST(observer.data() == owner.data());
  for (auto& v : owner) {
    v = 0;
  }
  for (const auto& p : owner.domain()) {
    BOOST_TEST(owner[p] == 0);
    BOOST_TEST(sharer[p] == 0);
    BOOST_TEST(observer[p] == 0);
  }
}

BOOST_AUTO_TEST_CASE(alignedraster_alignment_test) {
  constexpr Index width = 3;
  constexpr Index height = 4;
  AlignedRaster<int> aligned({width, height});
  BOOST_TEST(aligned.alignment() % 16 == 0);
  AlignedRaster<int> raw({width, height}, aligned.data() + 1, 1);
  BOOST_TEST(raw.alignment() == sizeof(int));
}

BOOST_AUTO_TEST_CASE(variable_dimension_raster_size_test) {
  const Index width = 4;
  const Index height = 3;
  const Index size = width * height;
  auto raster = random<int, -1>({width, height});
  BOOST_TEST(raster.dimension() == 2);
  BOOST_TEST(raster.size() == size);
}

BOOST_AUTO_TEST_CASE(subscript_bounds_test) {
  const Index width = 4;
  const Index height = 3;
  auto raster = random<int>({width, height});
  raster.at({1, -1}) = 1;
  BOOST_TEST(raster.at({1, -1}) == 1);
  const auto& vec = raster.data();
  BOOST_TEST((raster[{0, 0}]) == vec[0]);
  BOOST_TEST(raster.at({0, 0}) == vec[0]);
  BOOST_TEST(raster.at({-1, 0}) == vec[width - 1]);
  BOOST_TEST(raster.at({-width, 0}) == vec[0]);
  BOOST_TEST(raster.at({0, -1}) == vec[(height - 1) * width]);
  BOOST_TEST(raster.at({-1, -1}) == vec[height * width - 1]);
  BOOST_CHECK_THROW(raster.at({width, 0}), OutOfBoundsError);
  BOOST_CHECK_THROW(raster.at({-1 - width, 0}), OutOfBoundsError);
  BOOST_CHECK_THROW(raster.at({0, height}), OutOfBoundsError);
  BOOST_CHECK_THROW(raster.at({0, -1 - height}), OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(vecraster_move_test) {
  auto raster = random<int>({14, 3});
  const auto copied = raster.container();
  const auto data = raster.data();
  std::vector<int> moved;
  raster.moveTo(moved);
  BOOST_TEST(moved == copied);
  BOOST_TEST(moved.data() == data);
}

BOOST_AUTO_TEST_CASE(make_raster_test) {
  constexpr Index width = 16;
  constexpr Index height = 9;
  constexpr Index depth = 3;
  short data2[width * height] = {0};
  const short constData2[width * height] = {0};
  float data3[width * height * depth] = {0};
  const float constData3[width * height * depth] = {0};
  const auto raster2 = rasterize(data2, width, height);
  const auto constRaster2 = rasterize(constData2, width, height);
  const auto raster3 = rasterize(data3, width, height, depth);
  const auto constRaster3 = rasterize(constData3, width, height, depth);
  BOOST_TEST(raster2.dimension() == 2);
  BOOST_TEST(constRaster2.dimension() == 2);
  BOOST_TEST(raster3.dimension() == 3);
  BOOST_TEST(constRaster3.dimension() == 3);
}

BOOST_AUTO_TEST_CASE(slicing_test) {

  auto raster = random<float, 3>({5, 3, 4});

  // Several x-y planes
  Box<3> cube {{0, 0, 1}, {4, 2, 2}};
  BOOST_TEST(raster.isContiguous<3>(cube));
  const auto cubed = raster.slice<3>(cube);
  BOOST_TEST((cubed.shape() == Position<3>({5, 3, 2})));
  BOOST_TEST((cubed[{0, 0, 0}] == raster[cube.front()]));

  // One full x-y plane
  Box<3> plane {{0, 0, 1}, {4, 2, 1}};
  BOOST_TEST(raster.isContiguous<2>(plane));
  const auto planed = raster.slice<2>(plane);
  BOOST_TEST((planed.shape() == Position<2>({5, 3})));
  BOOST_TEST((planed[{0, 0}] == raster[plane.front()]));

  // One partial x-y plane
  Box<3> rectangle {{0, 1, 1}, {4, 2, 1}};
  BOOST_TEST(raster.isContiguous<2>(rectangle));
  const auto rectangled = raster.slice<2>(rectangle);
  BOOST_TEST((rectangled.shape() == Position<2>({5, 2})));
  BOOST_TEST((rectangled[{0, 0}] == raster[rectangle.front()]));

  // One partial x line
  Box<3> segment {{1, 1, 1}, {3, 1, 1}};
  BOOST_TEST(raster.isContiguous<1>(segment));
  const auto segmented = raster.slice<1>(segment);
  BOOST_TEST((segmented.shape() == Position<1>({3})));
  BOOST_TEST((segmented[{0}] == raster[segment.front()]));

  // Non-contiguous region
  Box<3> bad {{1, 1, 1}, {2, 2, 2}};
  BOOST_TEST(not raster.isContiguous<3>(bad));
  BOOST_CHECK_THROW(raster.slice<3>(bad), Exception);
}

BOOST_AUTO_TEST_CASE(sectionning_test) {

  const auto raster3D = random<short, 3>({8, 9, 12});

  // 3D
  const auto section3D = raster3D.section(3, 5);
  BOOST_TEST((section3D.shape() == Position<3>({8, 9, 3})));
  for (const auto& p : section3D.domain()) {
    BOOST_TEST((section3D[p] == raster3D[p + Position<3> {0, 0, 3}]));
  }

  // 2D
  const auto section2D = raster3D.section(3);
  BOOST_TEST(section2D.shape() == Position<2>({8, 9}));
  for (const auto& p : section2D.domain()) {
    BOOST_TEST((section2D[p] == raster3D[p.extend<3>({0, 0, 3})]));
  }

  // 1D
  const auto section1D = section2D.section(6);
  BOOST_TEST(section1D.shape() == Position<1> {8});
  for (const auto& p : section1D.domain()) {
    BOOST_TEST((section1D[p] == raster3D[p.extend<3>({0, 6, 3})]));
  }

  // 0D
  const auto section0D = section1D.section(2);
  BOOST_TEST(section0D.dimension() == 0);
  BOOST_TEST((*section0D.data() == raster3D[{2, 6, 3}]));
}

BOOST_AUTO_TEST_CASE(raster_row_test) {
  const auto raster = Raster<int>({6, 5}).range();
  const auto row0 = raster.row({0});
  const auto row1 = raster.row({1});
  const auto row4 = raster.row({-1});
  const auto width = raster.length<0>();
  BOOST_TEST(row0.size() == width);
  BOOST_TEST(row1.size() == width);
  BOOST_TEST(row4.size() == width);
  for (Index i = 0; i < width; ++i) {
    BOOST_TEST(row0[i] == (raster[{i, 0}]));
    BOOST_TEST(row1[i] == (raster[{i, 1}]));
    BOOST_TEST(row4[i] == (raster[{i, 4}]));
  }
}

BOOST_AUTO_TEST_CASE(raster_profile_test) {
  const auto raster = Raster<int, 3>({6, 5, 4}).range();
  const auto profile00 = raster.profile<1>({0, 0});
  const auto profile13 = raster.profile<1>({1, -1});
  const auto profile53 = raster.profile<1>({-1, -1});
  const auto height = raster.length<1>();
  BOOST_TEST(profile00.size() == height);
  BOOST_TEST(profile13.size() == height);
  BOOST_TEST(profile53.size() == height);
  for (Index i = 0; i < height; ++i) {
    BOOST_TEST(profile00[i] == (raster[{0, i, 0}]));
    BOOST_TEST(profile13[i] == (raster[{1, i, 3}]));
    BOOST_TEST(profile53[i] == (raster[{5, i, 3}]));
  }
}

BOOST_AUTO_TEST_CASE(raster_apply_generate_test) {
  Position<3> shape {3, 14, 15};
  auto a = random<std::int16_t>(shape);
  auto b = random<std::int32_t>(shape);
  VecRaster<std::int64_t, 3> result(shape);
  result.generate(
      [](auto v, auto w) {
        return v * w;
      },
      a,
      b);
  result.apply([](auto v) {
    return -v;
  });
  for (const auto& p : result.domain()) {
    BOOST_TEST((result[p] == -a[p] * b[p]));
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
