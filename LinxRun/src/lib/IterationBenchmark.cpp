// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "LinxRun/IterationBenchmark.h"

namespace Linx {

IterationBenchmark::IterationBenchmark(Index side) :
    m_width(side), m_height(side), m_depth(side), m_a({side, side, side}), m_b({side, side, side}),
    m_c({side, side, side})
{
  //! [Randomize]
  m_a.generate(UniformNoise<Value>(-50, 50));
  m_b.generate(GaussianNoise<Value>(0, 10));
  //! [Randomize]
}

IterationBenchmark::Duration IterationBenchmark::loop_over_xyz()
{
  m_timer.start();
  //! [x-y-z]
  for (Index x = 0; x < m_width; ++x) {
    for (Index y = 0; y < m_height; ++y) {
      for (Index z = 0; z < m_depth; ++z) {
        m_c[{x, y, z}] = m_a[{x, y, z}] + m_b[{x, y, z}];
      }
    }
  }
  //! [x-y-z]
  return m_timer.stop();
}

IterationBenchmark::Duration IterationBenchmark::loop_over_zyx()
{
  m_timer.start();
  //! [z-y-x]
  for (Index z = 0; z < m_depth; ++z) {
    for (Index y = 0; y < m_height; ++y) {
      for (Index x = 0; x < m_width; ++x) {
        m_c[{x, y, z}] = m_a[{x, y, z}] + m_b[{x, y, z}];
      }
    }
  }
  //! [z-y-x]
  return m_timer.stop();
}

IterationBenchmark::Duration IterationBenchmark::iterate_over_positions()
{
  m_timer.start();
  //! [position]
  for (const auto& p : m_c.domain()) {
    m_c[p] = m_a[p] + m_b[p];
  }
  //! [position]
  return m_timer.stop();
}

IterationBenchmark::Duration IterationBenchmark::iterate_over_positions_optimized()
{
  m_timer.start();
  //! [position-index]
  for (const auto& p : m_c.domain()) {
    const auto i = m_c.index(p);
    m_c[i] = m_a[i] + m_b[i];
  }
  //! [position-index]
  return m_timer.stop();
}

IterationBenchmark::Duration IterationBenchmark::loop_over_indices()
{
  m_timer.start();
  //! [index]
  const auto size = m_c.size();
  for (std::size_t i = 0; i < size; ++i) {
    m_c[i] = m_a[i] + m_b[i];
  }
  //! [index]
  return m_timer.stop();
}

IterationBenchmark::Duration IterationBenchmark::iterate_over_values()
{
  m_timer.start();
  //! [value]
  auto ait = m_a.begin();
  auto bit = m_b.begin();
  const auto begin = m_c.begin();
  const auto end = m_c.end();
  for (auto cit = begin; cit != end; ++cit, ++ait, ++bit) {
    *cit = *ait + *bit;
  }
  //! [value]
  return m_timer.stop();
}

IterationBenchmark::Duration IterationBenchmark::call_operator()
{
  m_timer.start();
  //! [operator]
  m_c = m_a + m_b;
  //! [operator]
  return m_timer.stop();
}

IterationBenchmark::Duration IterationBenchmark::call_generate()
{
  m_timer.start();
  //! [generate]
  m_c.generate(
      [](auto e, auto f) {
        return e + f;
      },
      m_a,
      m_b);
  //! [generate]
  return m_timer.stop();
}

} // namespace Linx
