// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Base/mixins/DataContainer.h"

#include <boost/test/unit_test.hpp>
#include <sstream>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(DataContainer_test)

//-----------------------------------------------------------------------------

void check_stream_insertion(MinimalDataContainer<int> container, const std::string& expected)
{
  std::stringstream os;
  os << container;
  BOOST_TEST(os.str() == expected);
}

BOOST_AUTO_TEST_CASE(stream_insertion_test)
{
  check_stream_insertion(MinimalDataContainer<int>(), "[]");
  check_stream_insertion({0}, "[0]");
  check_stream_insertion({0, 1}, "[0, 1]");
  check_stream_insertion({0, 1, 2}, "[0, 1, 2]");
  check_stream_insertion({0, 1, 2, 3}, "[0, 1, 2, 3]");
  check_stream_insertion({0, 1, 2, 3, 4}, "[0, 1, 2, 3, 4]");
  check_stream_insertion({0, 1, 2, 3, 4, 5}, "[0, 1, 2, 3, 4, 5]");
  check_stream_insertion({0, 1, 2, 3, 4, 5, 6}, "[0, 1, 2, 3, 4, 5, 6]");
  check_stream_insertion({0, 1, 2, 3, 4, 5, 6, 7}, "[0, 1, 2 ... 5, 6, 7]");
  check_stream_insertion({0, 1, 2, 3, 4, 5, 6, 7, 8}, "[0, 1, 2 ... 6, 7, 8]");
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
