// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Raster/AlignedBuffer.h"

#include <boost/test/unit_test.hpp>
#include <fftw3.h>

using namespace Cnes;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(AlignedBuffer_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(fftw_alloc_real_test) {
  double* p = fftw_alloc_real(10);
  BOOST_TEST(fftw_alignment_of(p) == 0);
  BOOST_TEST(fftw_alignment_of(p + 1) != 0);
  BOOST_TEST((std::uintptr_t(p) & 0xF) == 0);
  BOOST_TEST((std::uintptr_t(p + 1) & 0xF) != 0);
  fftw_free(p);
}

BOOST_AUTO_TEST_CASE(fftw_malloc_int_test) {
  int* p = (int*)fftw_malloc(sizeof(int) * 10);
  BOOST_TEST(fftw_alignment_of((double*)p) == 0);
  BOOST_TEST(fftw_alignment_of((double*)(p + 1)) != 0);
  BOOST_TEST((std::uintptr_t(p) & 0xF) == 0);
  BOOST_TEST((std::uintptr_t(p + 1) & 0xF) != 0);
  fftw_free(p);
}

BOOST_AUTO_TEST_CASE(aligned_alloc_test) {
  auto container = Internal::alignedAlloc<int>(10, nullptr, 16);
  int* p = container.second;
  BOOST_TEST(fftw_alignment_of((double*)p) == 0);
  BOOST_TEST(Internal::isAligned(p, 16));
  BOOST_TEST(Internal::alignment(p) % 16 == 0);
  auto view = Internal::alignedAlloc(10, p, 16);
  BOOST_TEST(fftw_alignment_of((double*)view.second) == 0);
  const auto cView = Internal::alignedAlloc<const int>(10, p, 16);
  BOOST_TEST(fftw_alignment_of((double*)cView.second) == 0);
  Internal::alignedFree(container);
  Internal::alignedFree(view);
  Internal::alignedFree(cView);
}

BOOST_AUTO_TEST_CASE(aligned_buffer_test) {
  for (std::size_t as = 16; as <= 1024; as <<= 1) {
    AlignedBuffer<int> buffer(10, nullptr, as);
    BOOST_TEST(fftw_alignment_of((double*)buffer.data()) == 0);
    BOOST_TEST(buffer.alignment() % as == 0);
    AlignedBuffer<int> view(10, const_cast<int*>(buffer.data()));
    BOOST_TEST(view.data() == buffer.data());
    AlignedBuffer<const int> cView(10, buffer.data());
    BOOST_TEST(cView.data() == buffer.data());
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
