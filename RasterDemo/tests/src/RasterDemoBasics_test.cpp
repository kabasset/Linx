// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Raster/Raster.h"

#include <boost/test/unit_test.hpp>

//! [MallocRaster]
template <typename T>
struct MallocContainer {

public:
  using Container = T*;

  MallocContainer(std::size_t s, T* d = nullptr) : m_size(s), m_container((T*)malloc(m_size * sizeof(T))) {
    if (d) {
      std::copy_n(d, m_size, m_container);
    }
  }

  ~MallocContainer() {
    free(m_container);
  }

  std::size_t size() const {
    return m_size;
  }

  const T* data() const {
    return m_container;
  }

private:
  std::size_t m_size;

  T* m_container;
};

template <typename T, Cnes::Index N>
using MallocRaster = Cnes::Raster<T, N, MallocContainer<T>>;
//! [MallocRaster]

Cnes::VecRaster<int, 3> vecRasterIota() {
  //! [VecRaster iota]
  Cnes::VecRaster<int, 3> raster({4, 3, 2});
  raster.arange(); // Assigns {0, 1, 2...}
  std::cout << "Access by ND position: " << raster[{2, 1, 0}] << std::endl; // 6
  std::cout << "Access by 1D index: " << raster[6] << std::endl; // 6
  //! [VecRaster iota]
  return raster;
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(RasterDemoBasics_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(vec_raster_iota_test) {
  vecRasterIota();
}

BOOST_AUTO_TEST_CASE(ptr_raster_iota_test) {
  //! [PtrRaster iota]
  std::array<int, 4 * 3 * 2> array;
  std::iota(array.begin(), array.end(), 0); // Same as raster.arange()
  Cnes::PtrRaster<int, 3> raster({4, 3, 2}, array.data());
  std::cout << "Access by ND position: " << raster[{2, 1, 0}] << std::endl; // 6
  raster *= 2;
  std::cout << "Updated value: " << array[6] << std::endl; // 12
  //! [PtrRaster iota]
}

BOOST_AUTO_TEST_CASE(malloc_raster_iota_test) {
  //! [MallocRaster iota]
  MallocRaster<int, 3> raster({4, 3, 2});
  raster.arange();
  std::cout << "Access by ND position: " << raster[{2, 1, 0}] << std::endl; // 6
  std::cout << "Access by 1D index: " << raster[6] << std::endl; // 6
  //! [MallocRaster iota]
}

BOOST_AUTO_TEST_CASE(element_access_test) {

  const auto raster = vecRasterIota();

  //! [Element access]
  std::cout << "Access by ND position: " << raster[{2, 2, 0}] << std::endl; // 10
  std::cout << "Access by 1D index: " << raster[10] << std::endl; // 10
  std::cout << "Backward indexing: " << raster.at({-2, -1, 0}) << std::endl; // 10
  //! [Element access]
}

BOOST_AUTO_TEST_CASE(foreach_element_test) {

  auto raster = vecRasterIota();

  //! [Foreach element]
  // Position
  for (Cnes::Index z = 0; z < raster.length(2); ++z) {
    for (Cnes::Index y = 0; y < raster.length(1); ++y) {
      for (Cnes::Index x = 0; x < raster.length(0); ++x) {
        raster[{x, y, z}] *= 2;
      }
    }
  }

  // Position iterator
  for (const auto& p : raster.domain()) {
    raster[p] *= 2;
  }

  // Index
  for (std::size_t i = 0; i < raster.size(); ++i) {
    raster[i] *= 2;
  }

  // Iterator (raw pointer)
  for (auto it = raster.begin(); it != raster.end(); ++it) {
    *it *= 2;
  }

  // Range for loop
  for (auto& e : raster) {
    e *= 2;
  }

  // Algorithm
  std::transform(raster.cbegin(), raster.cend(), raster.begin(), [](auto e) {
    return e * 2;
  });

  // operator*=
  raster *= 2;
  //! [Foreach element]
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
