// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RasterValidation/IterationBenchmark.h"

namespace Cnes {

IterationBenchmark::IterationBenchmark(Index side) :
    m_width(side), m_height(side), m_depth(side), m_a({side, side, side}), m_b({side, side, side}),
    m_c({side, side, side}) {}

IterationBenchmark::Duration IterationBenchmark::loopOverXyz() {
  m_chrono.start();
  //! [x-y-z]
  // Loop over X, Y, Z
  for (Index x = 0; x < m_width; ++x) {
    for (Index y = 0; y < m_height; ++y) {
      for (Index z = 0; z < m_depth; ++z) {
        m_c[{x, y, z}] = m_a[{x, y, z}] + m_b[{x, y, z}];
      }
    }
  }
  //! [x-y-z]
  return m_chrono.stop();
}

IterationBenchmark::Duration IterationBenchmark::loopOverZyx() {
  m_chrono.start();
  //! [z-y-x]
  // Loop over Z, Y, X
  for (Index z = 0; z < m_depth; ++z) {
    for (Index y = 0; y < m_height; ++y) {
      for (Index x = 0; x < m_width; ++x) {
        m_c[{x, y, z}] = m_a[{x, y, z}] + m_b[{x, y, z}];
      }
    }
  }
  //! [z-y-x]
  return m_chrono.stop();
}

IterationBenchmark::Duration IterationBenchmark::iterateOverPositions() {
  m_chrono.start();
  //! [position]
  // Iterate over positions
  for (const auto& p : m_c.domain()) {
    m_c[p] = m_a[p] + m_b[p];
  }
  //! [position]
  return m_chrono.stop();
}

IterationBenchmark::Duration IterationBenchmark::loopOverIndices() {
  m_chrono.start();
  //! [index]
  // Loop over indices
  const auto size = m_c.size();
  for (std::size_t i = 0; i < size; ++i) {
    m_c[i] = m_a[i] + m_b[i];
  }
  //! [index]
  return m_chrono.stop();
}

IterationBenchmark::Duration IterationBenchmark::iterateOverValues() {
  m_chrono.start();
  //! [value]
  // Iterate over values
  auto ait = m_a.begin();
  auto bit = m_b.begin();
  const auto begin = m_c.begin();
  const auto end = m_c.end();
  for (auto cit = begin; cit != end; ++cit, ++ait, ++bit) {
    *cit = *ait + *bit;
  }
  //! [value]
  return m_chrono.stop();
}

IterationBenchmark::Duration IterationBenchmark::callOperator() {
  m_chrono.start();
  //! [operator]
  // Call builtin operator
  m_c = m_a + m_b;
  //! [operator]
  return m_chrono.stop();
}

IterationBenchmark::Duration IterationBenchmark::callGenerate() {
  m_chrono.start();
  //! [generate]
  // Call Raster::generate()
  m_c.generate(
      [](auto e, auto f) {
        return e + f;
      },
      m_a,
      m_b);
  //! [generate]
  return m_chrono.stop();
}

} // namespace Cnes
