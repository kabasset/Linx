// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LinxRun/IterationBenchmark.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

struct BenchmarkFixture : IterationBenchmark {

  BenchmarkFixture() : IterationBenchmark(4) {}

  void validate() {
    const Position<3> shape {m_width, m_height, m_depth};
    const auto size = shapeSize(shape);
    BOOST_TEST(size > 0);
    BOOST_TEST(m_a.shape() == shape);
    BOOST_TEST(m_b.shape() == shape);
    BOOST_TEST(m_c.shape() == shape);
    for (Index i = 0; i < size; ++i) {
      BOOST_TEST(m_c[i] == Value(m_a[i] + m_b[i]));
    }
  }
};

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(IterationBenchmark_test, BenchmarkFixture)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(xyz_test) {
  loopOverXyz();
  validate();
}

BOOST_AUTO_TEST_CASE(zyx_test) {
  loopOverZyx();
  validate();
}

BOOST_AUTO_TEST_CASE(position_test) {
  iterateOverPositions();
  validate();
}

BOOST_AUTO_TEST_CASE(position_optimized_test) {
  iterateOverPositionsOptimized();
  validate();
}

BOOST_AUTO_TEST_CASE(index_test) {
  loopOverIndices();
  validate();
}

BOOST_AUTO_TEST_CASE(value_test) {
  iterateOverValues();
  validate();
}

BOOST_AUTO_TEST_CASE(operator_test) {
  callOperator();
  validate();
}

BOOST_AUTO_TEST_CASE(generate_test) {
  callGenerate();
  validate();
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
