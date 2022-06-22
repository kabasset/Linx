// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlContainer/ContiguousContainer.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(ContiguousContainer_test)

struct TestContiguousContainer : ContiguousContainerMixin<int, TestContiguousContainer> {

  TestContiguousContainer(std::size_t size = 0, const int* data = nullptr) : vector() {
    if (size > 0) {
      if (data) {
        vector.assign(data, data + size);
      } else {
        vector.resize(size);
      }
    }
  }

  const int* data() const {
    return vector.data();
  }

  std::size_t size() const {
    return vector.size();
  }

  std::vector<int> vector;
};

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(mixin_test) {

  constexpr std::size_t size = 10;
  TestContiguousContainer tcc(size);
  TestContiguousContainer empty;

  BOOST_TEST(not tcc.emtpy());
  BOOST_TEST(&tcc[size / 2] == &tcc.vector[size / 2]);
  BOOST_TEST(tcc.begin() == tcc.vector.data());
  BOOST_TEST(tcc.cbegin() == tcc.vector.data());
  BOOST_TEST(tcc.end() == tcc.vector.data() + size);
  BOOST_TEST(tcc.cend() == tcc.vector.data() + size);
  BOOST_TEST(tcc == tcc);
  BOOST_TEST(empty == empty);
  BOOST_TEST(tcc != empty);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
