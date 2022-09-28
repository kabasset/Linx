// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlBase/DataContainer.h"

#include <boost/test/unit_test.hpp>
#include <sstream>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(DataContainer_test)

//-----------------------------------------------------------------------------

void checkStreamInsertion(MinimalDataContainer<int> container, const std::string& expected) {
  std::stringstream os;
  os << container;
  BOOST_TEST(os.str() == expected);
}

BOOST_AUTO_TEST_CASE(stream_insertion_test) {
  checkStreamInsertion(MinimalDataContainer<int>(), "[]");
  checkStreamInsertion({0}, "[0]");
  checkStreamInsertion({0, 1}, "[0, 1]");
  checkStreamInsertion({0, 1, 2}, "[0, 1, 2]");
  checkStreamInsertion({0, 1, 2, 3}, "[0, 1, 2, 3]");
  checkStreamInsertion({0, 1, 2, 3, 4}, "[0, 1, 2, 3, 4]");
  checkStreamInsertion({0, 1, 2, 3, 4, 5}, "[0, 1, 2, 3, 4, 5]");
  checkStreamInsertion({0, 1, 2, 3, 4, 5, 6}, "[0, 1, 2, 3, 4, 5, 6]");
  checkStreamInsertion({0, 1, 2, 3, 4, 5, 6, 7}, "[0, 1, 2 ... 5, 6, 7]");
  checkStreamInsertion({0, 1, 2, 3, 4, 5, 6, 7, 8}, "[0, 1, 2 ... 6, 7, 8]");
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
