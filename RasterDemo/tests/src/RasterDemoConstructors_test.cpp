// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Raster/Raster.h"

#include <boost/test/unit_test.hpp>

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(RasterDemoConstructors_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(any_raster_ctors_test) {

  //! [Any raster]
  std::vector<int> vec {1, 2, 3, 4, 5, 6};
  const int* data = vec.data();

  Cnes::VecRaster<int> defaultInitialized({3, 2});
  Cnes::VecRaster<int> listInitialized({3, 2}, {1, 2, 3, 4, 5, 6});
  Cnes::VecRaster<int> copiedFromPointer({3, 2}, data);
  Cnes::VecRaster<int> copiedFromIterable({3, 2}, vec);
  Cnes::VecRaster<int> movedFromContainer({3, 2}, std::move(vec));
  //! [Any raster]

  for (std::size_t i = 0; i < 6; ++i) {
    BOOST_TEST(defaultInitialized[i] == 0);
    BOOST_TEST(listInitialized[i] == i + 1);
    BOOST_TEST(copiedFromPointer[i] == i + 1);
    BOOST_TEST(copiedFromIterable[i] == i + 1);
    BOOST_TEST(movedFromContainer[i] == i + 1);
  }
  BOOST_TEST(movedFromContainer.data() == data);

  const int* listData = listInitialized.data();
  //! [Any raster copy-move]
  Cnes::VecRaster<int> copiedFromRaster(listInitialized);
  Cnes::VecRaster<int> movedFromRaster(std::move(listInitialized));
  //! [Any raster copy-move]

  for (std::size_t i = 0; i < 6; ++i) {
    BOOST_TEST(copiedFromRaster[i] == i + 1);
    BOOST_TEST(movedFromRaster[i] == i + 1);
  }
  BOOST_TEST(movedFromRaster.data() == listData);
}

BOOST_AUTO_TEST_CASE(ptrraster_ctors_test) {

  //! [PtrRaster write]
  int data[] = {1, 2, 3, 4, 5, 6};
  Cnes::PtrRaster<int> constructed({3, 2}, data);
  auto made = Cnes::makeRaster(data, 3, 2);

  constructed[0] = 42;
  made[1] = 12;

  BOOST_TEST(constructed[0] == 42);
  BOOST_TEST(constructed[1] == 12);
  BOOST_TEST(made[0] == 42);
  BOOST_TEST(made[1] == 12);
  //! [PtrRaster write]

  //! [PtrRaster read]
  Cnes::PtrRaster<const int> cConstructed({3, 2}, data);
  const int* cData = data;
  auto cMade = Cnes::makeRaster(cData, 3, 2);

  BOOST_TEST((made == constructed));
  BOOST_TEST((cMade == cConstructed));
  //! [PtrRaster read]
}

BOOST_AUTO_TEST_CASE(vecraster_ctors_test) {
  //! [VecRaster]
  std::vector<int> vec {1, 2, 3, 4, 5, 6};
  const int* data = vec.data();

  Cnes::VecRaster<int> copied({3, 2}, data); // Copy data
  BOOST_TEST(copied[0] == 1);
  Cnes::VecRaster<int> moved({3, 2}, std::move(vec)); // Move container
  BOOST_TEST(moved[0] == 1);
  BOOST_TEST(moved.data() == data);
  Cnes::VecRaster<int> filled({3, 2}, 42); // Fill with a single value
  BOOST_TEST(filled[0] == 42);
  Cnes::VecRaster<int> listed({3, 2}, {12, 10, 8, 6, 4, 2}); // Fill with a list of values
  BOOST_TEST(listed[0] == 12);
  //! [VecRaster]
}

BOOST_AUTO_TEST_CASE(alignedraster_ctors_test) {

  //! [AlignedRaster owns]
  Cnes::AlignedRaster<int> defaultAligned({3, 2}); // 16 byte-aligned for SIMD
  Cnes::AlignedRaster<int> longerAligned({3, 2}, nullptr, 64); // Longer alignment
  BOOST_TEST(defaultAligned.alignment() % 16 == 0);
  BOOST_TEST(longerAligned.alignment() % 64 == 0);
  //! [AlignedRaster owns]

  //! [AlignedRaster shares]
  int data[] = {1, 2, 3, 4, 5, 6};

  Cnes::AlignedRaster<int> notAligned({3, 2}, data, 1); // No alignment required
  BOOST_TEST(notAligned.data() == data);

  try {
    Cnes::AlignedRaster<int> maybeAligned({3, 2}, data, 64); // Alignment required
    std::cout << "Data is aligned!" << std::endl;
  } catch (Cnes::Exception& e) { // FIXME dedicated error
    BOOST_TEST(notAligned.alignment() % 64 != 0); // Alignment requirement not met
    std::cout << "Data is not aligned!" << std::endl;
  }
  //! [AlignedRaster shares]
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
