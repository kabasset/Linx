// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RasterFourier/Dft.h"
#include "RasterFourier/DftMemory.h"

#include <boost/test/unit_test.hpp>

using namespace Cnes;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(DftMemory_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(allocate_plan_test) {
  const Position<3> shape {6, 8, 2};
  const Position<3> halfShape {3, 8, 2};
  RealDftBuffer<3> rin(shape);
  RealDftBuffer<3> rout(shape);
  ComplexDftBuffer<3> cin(shape);
  ComplexDftBuffer<3> cout(shape);
  auto rc = FftwAllocator::createPlan<Internal::RealDftTransform>(rin, cout);
  BOOST_TEST(rc.get() != nullptr);
  FftwAllocator::destroyPlan(rc);
  auto irc = FftwAllocator::createPlan<Internal::Inverse<Internal::RealDftTransform>>(cout, rin);
  BOOST_TEST(irc.get() != nullptr);
  FftwAllocator::destroyPlan(irc);
  auto cc = FftwAllocator::createPlan<Internal::ComplexDftTransform>(cin, cout);
  BOOST_TEST(cc.get() != nullptr);
  FftwAllocator::destroyPlan(cc);
  auto icc = FftwAllocator::createPlan<Internal::Inverse<Internal::ComplexDftTransform>>(cout, cin);
  BOOST_TEST(icc.get() != nullptr);
  FftwAllocator::destroyPlan(icc);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
