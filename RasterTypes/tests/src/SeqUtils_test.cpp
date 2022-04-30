// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RasterTypes/SeqUtils.h"

#include <boost/test/unit_test.hpp>
#include <type_traits>

using namespace Cnes;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(SeqUtils_test)

//-----------------------------------------------------------------------------

CNES_RASTER_TEST_CASE_TEMPLATE(supported_types_test) {
  BOOST_TEST(std::is_standard_layout<T>::value);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
