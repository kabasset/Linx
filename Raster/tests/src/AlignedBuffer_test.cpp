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

BOOST_AUTO_TEST_CASE(alignment_test) {
  for (std::size_t as = 16; as <= 1024; as <<= 1) {
    AlignedBuffer<int> buffer(10, nullptr, as);
    BOOST_TEST(fftw_alignment_of((double*)buffer.data()) == 0);
    BOOST_TEST(buffer.alignmentReq() == as);
    BOOST_TEST(buffer.alignment() % as == 0);
    AlignedBuffer<int> view(10, const_cast<int*>(buffer.data()), as);
    BOOST_TEST(view.data() == buffer.data());
    AlignedBuffer<const int> cView(10, buffer.data(), as);
    BOOST_TEST(cView.data() == buffer.data());
  }
}

BOOST_AUTO_TEST_CASE(release_reset_test) {
  AlignedBuffer<int> owner(7);
  AlignedBuffer<const int> viewer(7, owner.data());
  BOOST_TEST(not viewer.release());
  BOOST_TEST(viewer.data());
  auto* p = owner.release();
  BOOST_TEST(p);
  BOOST_TEST(owner.data());
  BOOST_TEST(not owner.owns());
  BOOST_CHECK_NO_THROW(std::free(p));
  viewer.reset();
  BOOST_TEST(not viewer.data());
  owner.reset();
  BOOST_TEST(not owner.data());
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
