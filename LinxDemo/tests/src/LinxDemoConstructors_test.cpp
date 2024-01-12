// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Data/Raster.h"

#include <boost/test/unit_test.hpp>

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(LinxDemoConstructors_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(any_raster_ctors_test)
{
  //! [Any raster]
  std::vector<int> vec {1, 2, 3, 4, 5, 6};
  const int* data = vec.data();

  Linx::VecRaster<int> default_initialized({3, 2});
  Linx::VecRaster<int> list_initialized({3, 2}, {1, 2, 3, 4, 5, 6});
  Linx::VecRaster<int> copied_from_pointer({3, 2}, data);
  Linx::VecRaster<int> copied_from_range({3, 2}, vec);
  Linx::VecRaster<int> moved_from_container({3, 2}, std::move(vec));
  //! [Any raster]

  for (std::size_t i = 0; i < 6; ++i) {
    BOOST_TEST(default_initialized[i] == 0);
    BOOST_TEST(list_initialized[i] == i + 1);
    BOOST_TEST(copied_from_pointer[i] == i + 1);
    BOOST_TEST(copied_from_range[i] == i + 1);
    BOOST_TEST(moved_from_container[i] == i + 1);
  }
  BOOST_TEST(copied_from_range.data() != data);
  BOOST_TEST(moved_from_container.data() == data);

  const int* list_data = list_initialized.data();
  //! [Any raster copy-move]
  Linx::VecRaster<int> copied_from_raster(list_initialized);
  Linx::VecRaster<int> moved_from_raster(std::move(list_initialized));
  //! [Any raster copy-move]

  for (std::size_t i = 0; i < 6; ++i) {
    BOOST_TEST(copied_from_raster[i] == i + 1);
    BOOST_TEST(moved_from_raster[i] == i + 1);
  }
  BOOST_TEST(moved_from_raster.data() == list_data);
}

BOOST_AUTO_TEST_CASE(vecraster_move_test)
{
  //! [VecRaster moves]
  std::vector<int> vec {1, 2, 3, 4, 5, 6};
  const auto data = vec.data();

  Linx::VecRaster<int> raster({3, 2}, std::move(vec)); // Transfer ownership to raster, no copy
  BOOST_TEST(raster.data() == data);

  const std::vector<int>& ref = raster.container(); // Access as const reference, no copy
  BOOST_TEST(ref.data() == data);

  raster.move_to(vec); // Transfer back ownership to vector, no copy
  BOOST_TEST(vec.data() == data);
  //! [VecRaster moves]
}

BOOST_AUTO_TEST_CASE(ptrraster_ctors_test)
{
  //! [PtrRaster write]
  int data[] = {1, 2, 3, 4, 5, 6};

  Linx::PtrRaster<int> constructed({3, 2}, data);
  auto made = Linx::rasterize(data, 3, 2);

  constructed[0] = 42;
  made[1] = 12;

  BOOST_TEST((made == constructed));
  //! [PtrRaster write]

  BOOST_TEST(constructed[0] == 42);
  BOOST_TEST(constructed[1] == 12);
  BOOST_TEST(made[0] == 42);
  BOOST_TEST(made[1] == 12);

  //! [PtrRaster read]
  const int* c_data = data;

  Linx::PtrRaster<const int> c_constructed({3, 2}, data);
  auto c_made = Linx::rasterize(c_data, 3, 2);

  BOOST_TEST((c_made == c_constructed));
  //! [PtrRaster read]
}

BOOST_AUTO_TEST_CASE(alignedraster_ctors_test)
{
  //! [AlignedRaster owns]
  Linx::AlignedRaster<int> default_aligned({3, 2}); // SIMD-aligned
  BOOST_TEST(default_aligned.alignment() % 16 == 0);
  BOOST_TEST(default_aligned.owns());

  Linx::AlignedRaster<int> longer_aligned({3, 2}, nullptr, 1024); // Longer alignment
  BOOST_TEST(longer_aligned.alignment() % 1024 == 0);
  BOOST_TEST(longer_aligned.owns());
  //! [AlignedRaster owns]

  //! [AlignedRaster shares]
  int data[] = {1, 2, 3, 4, 5, 6}; // Unknown alignment

  Linx::AlignedRaster<int> not_aligned({3, 2}, data); // No alignment required
  BOOST_TEST(not_aligned.data() == data);
  BOOST_TEST(not not_aligned.owns());

  try {
    Linx::AlignedRaster<int> maybe_aligned({3, 2}, data, 64); // Alignment required
    std::cout << "Data is aligned!" << std::endl;
    BOOST_TEST(not maybe_aligned.owns());

  } catch (Linx::AlignmentError& e) { // Alignment requirement not met
    BOOST_TEST(not_aligned.alignment() % 64 != 0);
    std::cout << e.what() << std::endl;
  }
  //! [AlignedRaster shares]
}

BOOST_AUTO_TEST_CASE(fill_test)
{
  //! [Filling]
  Linx::AlignedRaster<double> raster({3, 2});

  // Single value
  raster.fill(42);
  std::cout << raster << std::endl; // [42, 42, 42...]
  BOOST_TEST(raster[0] == 42);

  // Evenly spaced values (from min, step)
  raster.range(1, 2);
  std::cout << raster << std::endl; // [1, 3, 5...]
  BOOST_TEST(raster[0] == 1);

  // Evenly spaced values (from min, max)
  raster.linspace(0, Linx::pi<double>());
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
