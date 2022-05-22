// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Raster/Raster.h"

#include <boost/test/unit_test.hpp>

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(RasterDemoConstructors_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(ptrraster_ctors_test) {
  //! [PtrRaster data]
  //! [PtrRaster data]

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
  //! [VecRaster data]
  std::vector<int> vec {1, 2, 3, 4, 5, 6};
  int* data = vec.data();
  //! [VecRaster data]

  //! [VecRaster ctor]
  Cnes::VecRaster<int> copied({3, 2}, data); // Copy data
  BOOST_TEST(copied[0] == 1);
  Cnes::VecRaster<int> moved({3, 2}, std::move(vec)); // Move container
  BOOST_TEST(moved[0] == 1);
  BOOST_TEST(moved.data() == data);
  Cnes::VecRaster<int> filled({3, 2}, 42); // Fill with a single value
  BOOST_TEST(filled[0] == 42);
  Cnes::VecRaster<int> listed({3, 2}, {12, 10, 8, 6, 4, 2}); // Fill with a list of values
  BOOST_TEST(listed[0] == 12);
  //! [VecRaster ctor]
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
