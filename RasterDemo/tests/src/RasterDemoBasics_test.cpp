// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Raster/Raster.h"

#include <boost/test/unit_test.hpp>

//! [MallocRaster]
template <typename T>
struct MallocHolder {

public:
  MallocHolder(std::size_t s, T* d = nullptr) : m_size(s), m_data((T*)malloc(m_size * sizeof(T))) {
    if (d) {
      std::copy_n(d, m_size, m_data);
    }
  }

  ~MallocHolder() {
    free(m_data);
  }

  std::size_t size() const {
    return m_size;
  }

  inline const T* data() const {
    return m_data;
  }

private:
  std::size_t m_size;
  T* m_data;
};

template <typename T, Cnes::Index N>
using MallocRaster = Cnes::Raster<T, N, MallocHolder<T>>;
//! [MallocRaster]

Cnes::VecRaster<int, 3> rasterIota() {
  //! [Raster iota]
  Cnes::Raster<int, 3> raster({4, 3, 2});
  raster.range(); // Assigns {0, 1, 2...}
  std::cout << "Access by ND position: " << raster[{2, 1, 0}] << std::endl; // 6
  std::cout << "Access by 1D index: " << raster[6] << std::endl; // 6
  //! [Raster iota]
  return raster;
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(RasterDemoBasics_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(raster_iota_test) {
  rasterIota();
}

BOOST_AUTO_TEST_CASE(vec_raster_iota_test) {
  //! [VecRaster iota]
  Cnes::VecRaster<int, 3> raster({4, 3, 2});
  raster.range(); // Assigns {0, 1, 2...}
  std::cout << "Access by ND position: " << raster[{2, 1, 0}] << std::endl; // 6
  std::cout << "Access by 1D index: " << raster[6] << std::endl; // 6
  //! [VecRaster iota]
}

BOOST_AUTO_TEST_CASE(ptr_raster_iota_test) {
  //! [PtrRaster iota]
  std::array<int, 4 * 3 * 2> array;
  std::iota(array.begin(), array.end(), 0); // Same as raster.range()
  Cnes::PtrRaster<int, 3> raster({4, 3, 2}, array.data());
  std::cout << "Access by ND position: " << raster[{2, 1, 0}] << std::endl; // 6
  raster *= 2;
  std::cout << "Updated value: " << array[6] << std::endl; // 12
  //! [PtrRaster iota]
}

BOOST_AUTO_TEST_CASE(malloc_raster_iota_test) {
  //! [MallocRaster iota]
  MallocRaster<int, 3> raster({4, 3, 2});
  raster.range();
  std::cout << "Access by ND position: " << raster[{2, 1, 0}] << std::endl; // 6
  std::cout << "Access by 1D index: " << raster[6] << std::endl; // 6
  //! [MallocRaster iota]
}

BOOST_AUTO_TEST_CASE(element_access_test) {

  const auto raster = rasterIota();

  //! [Element access]
  std::cout << "Access by ND position: " << raster[{2, 2, 0}] << std::endl; // 10
  std::cout << "Access by 1D index: " << raster[10] << std::endl; // 10
  std::cout << "Backward indexing: " << raster.at({-2, -1, 0}) << std::endl; // 10
  //! [Element access]
}

BOOST_AUTO_TEST_CASE(foreach_element_test) {

  auto raster = rasterIota();

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
