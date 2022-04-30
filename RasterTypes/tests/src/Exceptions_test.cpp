// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RasterTypes/Exceptions.h"

#include <boost/test/unit_test.hpp>

using namespace Cnes;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Exceptions_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(message_test) {
  const std::string prefix = "Raster";
  const std::string message = "MESSAGE!";
  Exception error(message);
  const std::string output = error.what();
  const auto prefixPos = output.find(prefix);
  BOOST_TEST(prefixPos != std::string::npos);
  const auto messagePos = output.find(message, prefixPos + prefix.length());
  BOOST_TEST(messagePos != std::string::npos);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
