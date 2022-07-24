// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlContainer/Sequence.h"
#include "LitlTypes/TypeUtils.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Sequence_test)

//-----------------------------------------------------------------------------

LITL_TEST_CASE_TEMPLATE(element_access_test) {
  T data[] = {T(0), T(1), T(0), T(1), T(2), T(3)};
  Sequence<T> seq(6, data);
  BOOST_TEST(seq.size() == 6);
  for (std::size_t i = 0; i < 6; ++i) {
    BOOST_TEST(seq[i] == data[i]);
  }
  seq[0] = T(1);
  BOOST_TEST(seq[0] == T(1));
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
