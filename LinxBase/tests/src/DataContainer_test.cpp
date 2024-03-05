// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Base/mixins/DataContainer.h"
#include "Linx/Data/Sequence.h"

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

BOOST_AUTO_TEST_CASE(empty_container_contains_no_given_value_test)
{
  Sequence<int> container {};
  BOOST_TEST(not container.contains(0));
  BOOST_TEST(not container.contains_only(0));
  BOOST_TEST(not container.contains_nan());
}

BOOST_AUTO_TEST_CASE(singleton_contains_only_its_value_test)
{
  Sequence<int> container {1};
  BOOST_TEST(container.contains(1));
  BOOST_TEST(container.contains_only(1));
  BOOST_TEST(not container.contains(0));
  BOOST_TEST(not container.contains_nan());
}

BOOST_AUTO_TEST_CASE(constant_container_contains_only_its_value_test)
{
  Sequence<int> container {3, 3, 3};
  BOOST_TEST(container.contains(3));
  BOOST_TEST(container.contains_only(3));
  BOOST_TEST(not container.contains(0));
  BOOST_TEST(not container.contains_nan());
}

BOOST_AUTO_TEST_CASE(singleton_contains_nan_test)
{
  auto nan = std::numeric_limits<float>::quiet_NaN();
  Sequence<float> container {nan};
  BOOST_TEST(not container.contains(nan));
  BOOST_TEST(container.contains_nan());
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
