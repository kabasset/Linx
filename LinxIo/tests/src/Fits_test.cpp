/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LinxIo/Fits.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Fits_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(write_read_test) { // FIXME foreach type, for several Ns
  Raster<char> in({16, 16});
  in.range();
  Fits io("test.fits");
  io.write(in);
  auto out = io.read<Raster<char>>();
  BOOST_TEST(out == in);
  remove("test.fits");
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
