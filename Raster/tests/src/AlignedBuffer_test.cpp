// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Raster/AlignedBuffer.h"

#include <boost/test/unit_test.hpp>

using namespace Cnes;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(AlignedBuffer_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(fftw_alloc_real_test) {
  double* p = fftw_alloc_real(10);
  BOOST_TEST(fftw_alignment_of(p) == 0);
  BOOST_TEST(fftw_alignment_of(p + 1) != 0);
  BOOST_TEST((intptr_t(p) & 0xF) == 0);
  BOOST_TEST((intptr_t(p + 1) & 0xF) != 0);
  fftw_free(p);
}

BOOST_AUTO_TEST_CASE(fftw_malloc_int_test) {
  int* p = (int*)fftw_malloc(sizeof(int) * 10);
  BOOST_TEST(fftw_alignment_of((double*)p) == 0);
  BOOST_TEST(fftw_alignment_of((double*)(p + 1)) != 0);
  BOOST_TEST((intptr_t(p) & 0xF) == 0);
  BOOST_TEST((intptr_t(p + 1) & 0xF) != 0);
  fftw_free(p);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
