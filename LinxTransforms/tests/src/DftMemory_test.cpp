// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Transforms/Dft.h"
#include "Linx/Transforms/DftMemory.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(DftMemory_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(fftw_alloc_real_test)
{
  double* p = fftw_alloc_real(10);
  BOOST_TEST(fftw_alignment_of(p) == 0);
  BOOST_TEST(fftw_alignment_of(p + 1) != 0);
  BOOST_TEST((std::uintptr_t(p) & 0xF) == 0);
  BOOST_TEST((std::uintptr_t(p + 1) & 0xF) != 0);
  fftw_free(p);
}

BOOST_AUTO_TEST_CASE(fftw_malloc_int_test)
{
  int* p = (int*)fftw_malloc(sizeof(int) * 10);
  BOOST_TEST(fftw_alignment_of((double*)p) == 0);
  BOOST_TEST(fftw_alignment_of((double*)(p + 1)) != 0);
  BOOST_TEST((std::uintptr_t(p) & 0xF) == 0);
  BOOST_TEST((std::uintptr_t(p + 1) & 0xF) != 0);
  fftw_free(p);
}

BOOST_AUTO_TEST_CASE(allocate_plan_test)
{
  const Position<3> shape {6, 8, 2};
  RealDftBuffer<3> rin(shape);
  RealDftBuffer<3> rout(shape);
  ComplexDftBuffer<3> cin(shape);
  ComplexDftBuffer<3> cout(shape);
  auto rc = FftwAllocator::create_plan<Internal::RealDftTransform>(rin, cout);
  BOOST_TEST(rc.get() != nullptr);
  FftwAllocator::destroy_plan(rc);
  auto irc = FftwAllocator::create_plan<Internal::Inverse<Internal::RealDftTransform>>(cout, rin);
  BOOST_TEST(irc.get() != nullptr);
  FftwAllocator::destroy_plan(irc);
  auto cc = FftwAllocator::create_plan<Internal::ComplexDftTransform>(cin, cout);
  BOOST_TEST(cc.get() != nullptr);
  FftwAllocator::destroy_plan(cc);
  auto icc = FftwAllocator::create_plan<Internal::Inverse<Internal::ComplexDftTransform>>(cout, cin);
  BOOST_TEST(icc.get() != nullptr);
  FftwAllocator::destroy_plan(icc);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
