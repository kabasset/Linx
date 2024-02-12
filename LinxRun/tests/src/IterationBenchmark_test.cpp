// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "LinxRun/IterationBenchmark.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

struct BenchmarkFixture : IterationBenchmark {
  BenchmarkFixture() : IterationBenchmark(4) {}

  void validate()
  {
    const Position<3> shape {m_width, m_height, m_depth};
    const auto size = shape_size(shape);
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

BOOST_AUTO_TEST_CASE(xyz_test)
{
  loop_over_xyz();
  validate();
}

BOOST_AUTO_TEST_CASE(zyx_test)
{
  loop_over_zyx();
  validate();
}

BOOST_AUTO_TEST_CASE(position_test)
{
  iterate_over_positions();
  validate();
}

BOOST_AUTO_TEST_CASE(position_optimized_test)
{
  iterate_over_positions_optimized();
  validate();
}

BOOST_AUTO_TEST_CASE(index_test)
{
  loop_over_indices();
  validate();
}

BOOST_AUTO_TEST_CASE(value_test)
{
  iterate_over_values();
  validate();
}

BOOST_AUTO_TEST_CASE(operator_test)
{
  call_operator();
  validate();
}

BOOST_AUTO_TEST_CASE(generate_test)
{
  call_generate();
  validate();
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
