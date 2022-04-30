// Copyright (C) 2022, CNES
// This file is part of KRaster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "KRasterFourier/Dft.h"
#include "KRasterFourier/DftBuffer.h"
#include "KRasterFourier/DftMemory.h"

#include <boost/test/unit_test.hpp>

using namespace Cnes;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(DftMemory_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(allocate_plan_test) {
  const Position<2> shape {6, 8};
  const Position<2> halfShape {3, 8};
  RealDftBuffer rin(shape);
  RealDftBuffer rout(shape);
  ComplexDftBuffer cin(shape);
  ComplexDftBuffer cout(shape);
  auto rc = FftwAllocator::createPlan<RealDftType>(rin, cout);
  BOOST_TEST(rc.get() != nullptr);
  FftwAllocator::destroyPlan(rc);
  auto irc = FftwAllocator::createPlan<Inverse<RealDftType>>(cout, rin);
  BOOST_TEST(irc.get() != nullptr);
  FftwAllocator::destroyPlan(irc);
  auto cc = FftwAllocator::createPlan<ComplexDftType>(cin, cout);
  BOOST_TEST(cc.get() != nullptr);
  FftwAllocator::destroyPlan(cc);
  auto icc = FftwAllocator::createPlan<Inverse<ComplexDftType>>(cout, cin);
  BOOST_TEST(icc.get() != nullptr);
  FftwAllocator::destroyPlan(icc);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
