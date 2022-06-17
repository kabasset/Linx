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
  BOOST_TEST(copiedFromIterable.data() != data);
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

BOOST_AUTO_TEST_CASE(vecraster_move_test) {

  //! [VecRaster moves]
  std::vector<int> vec {1, 2, 3, 4, 5, 6};
  const auto data = vec.data();

  Cnes::VecRaster<int> raster({3, 2}, std::move(vec)); // Transfer ownership to raster, no copy
  BOOST_TEST(raster.data() == data);

  const std::vector<int>& ref = raster.container(); // Access as const reference, no copy
  BOOST_TEST(ref.data() == data);

  raster.moveTo(vec); // Transfer back ownership to vector, no copy
  BOOST_TEST(vec.data() == data);
  //! [VecRaster moves]
}

BOOST_AUTO_TEST_CASE(ptrraster_ctors_test) {

  //! [PtrRaster write]
  int data[] = {1, 2, 3, 4, 5, 6};

  Cnes::PtrRaster<int> constructed({3, 2}, data);
  auto made = Cnes::rasterize(data, 3, 2);

  constructed[0] = 42;
  made[1] = 12;

  BOOST_TEST((made == constructed));
  //! [PtrRaster write]

  BOOST_TEST(constructed[0] == 42);
  BOOST_TEST(constructed[1] == 12);
  BOOST_TEST(made[0] == 42);
  BOOST_TEST(made[1] == 12);

  //! [PtrRaster read]
  const int* cData = data;

  Cnes::PtrRaster<const int> cConstructed({3, 2}, data);
  auto cMade = Cnes::rasterize(cData, 3, 2);

  BOOST_TEST((cMade == cConstructed));
  //! [PtrRaster read]
}

BOOST_AUTO_TEST_CASE(alignedraster_ctors_test) {

  //! [AlignedRaster owns]
  Cnes::AlignedRaster<int> defaultAligned({3, 2}); // SIMD-aligned
  BOOST_TEST(defaultAligned.alignment() % 16 == 0);
  BOOST_TEST(defaultAligned.owns());

  Cnes::AlignedRaster<int> longerAligned({3, 2}, nullptr, 1024); // Longer alignment
  BOOST_TEST(longerAligned.alignment() % 1024 == 0);
  BOOST_TEST(longerAligned.owns());
  //! [AlignedRaster owns]

  //! [AlignedRaster shares]
  int data[] = {1, 2, 3, 4, 5, 6}; // Unknown alignment

  Cnes::AlignedRaster<int> notAligned({3, 2}, data); // No alignment required
  BOOST_TEST(notAligned.data() == data);
  BOOST_TEST(not notAligned.owns());

  try {
    Cnes::AlignedRaster<int> maybeAligned({3, 2}, data, 64); // Alignment required
    std::cout << "Data is aligned!" << std::endl;
    BOOST_TEST(not maybeAligned.owns());

  } catch (Cnes::AlignmentError& e) { // Alignment requirement not met
    BOOST_TEST(notAligned.alignment() % 64 != 0);
    std::cout << e.what() << std::endl;
  }
  //! [AlignedRaster shares]
}

BOOST_AUTO_TEST_CASE(fill_test) {

  //! [Filling]
  Cnes::AlignedRaster<double> raster({3, 2});

  // Single value
  raster.fill(42);
  std::cout << raster << std::endl; // [42, 42, 42...]
  BOOST_TEST(raster[0] == 42);

  // Evenly spaced values (from min, step)
  raster.arange(1, 2);
  std::cout << raster << std::endl; // [1, 3, 5...]
  BOOST_TEST(raster[0] == 1);

  // Evenly spaced values (from min, max)
  raster.linspace(0, Cnes::pi<double>());
  std::cout << raster << std::endl; // [0, 0.2 π, 0.4 π...]
  BOOST_TEST(raster[0] == 0);

  // Value generation
  bool toggle = false;
  raster.generate([&]() {
    toggle = not toggle;
    return toggle;
  });
  std::cout << raster << std::endl; // [1, 0, 1...]
  BOOST_TEST(raster[0] == 1);

  // Value transformation
  raster.apply([](bool e) {
    return not e;
  });
  std::cout << raster << std::endl; // [0, 1, 0...]
  BOOST_TEST(raster[0] == 0);
  //! [Filling]
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
