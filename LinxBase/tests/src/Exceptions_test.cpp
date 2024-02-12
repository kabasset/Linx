// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Base/Exceptions.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Exceptions_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(message_test)
{
  const std::string prefix = "Linx";
  const std::string message = "MESSAGE!";
  Exception error(message);
  const std::string output = error.what();
  const auto prefix_pos = output.find(prefix);
  BOOST_TEST(prefix_pos != std::string::npos);
  const auto message_pos = output.find(message, prefix_pos + prefix.length());
  BOOST_TEST(message_pos != std::string::npos);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
