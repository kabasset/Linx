// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRUN_CHRONOMETER_H
#define _LITLRUN_CHRONOMETER_H

#include <algorithm> // accumulate, min_element, max_element
#include <chrono>
#include <cmath> // sqrt
#include <numeric> // inner_product
#include <vector>

namespace Litl {

/**
 * @brief A simple chronometer with increment times and elapsed time caching.
 * @tparam TUnit The time unit, e.g. `std::chrono::milliseconds`
 * 
 * Each time the chronometer is started and stoped, an increment is computed,
 * and the total elapsed time is incremented.
 * An offset can be provided, which is the initial value of the elapsed time,
 * but has no effect on the increments.
 *
 * Simple statistics on the increments can be computed (e.g. mean increment).
 *
 * The chronometer can be reset, which means that the list of increments is emptied,
 * and the elapsed time is set to 0 or the offset.
 */
template <typename TUnit>
class Chronometer { // FIXME make a DataContainer and get StatsMixin
public:
  /**
   * @brief The time unit.
   */
  using Unit = TUnit;

  /**
   * @brief Create a chronometer with optional offset.
   */
  Chronometer(TUnit offset = TUnit()) : m_tic(), m_toc(), m_running(false), m_container(), m_elapsed(offset) {
    reset(offset);
  }

  /**
   * @brief Reset the chronometer with optional offset.
   */
  void reset(TUnit offset = TUnit()) {
    m_toc = m_tic;
    m_running = false;
    m_container.resize(0);
    m_elapsed = offset;
  }

  /**
   * @brief Start or restart the chronometer.
   */
  void start() {
    m_tic = std::chrono::steady_clock::now();
    m_running = true;
  }

  /**
   * @brief Stop the chronometer and get the last time increment.
   */
  TUnit stop() {
    m_toc = std::chrono::steady_clock::now();
    m_running = false;
    const auto inc = std::chrono::duration_cast<TUnit>(m_toc - m_tic);
    m_elapsed += inc;
    m_container.push_back(inc.count());
    return inc;
  }

  /**
   * @brief Test whether the chronometer is running.
   */
  bool isRunning() const {
    return m_running; // TODO m_running could be removed by comparing m_toc to m_tic, but is it relevant?
  }

  /**
   * @brief The last increment.
   */
  TUnit last() const {
    return TUnit {typename TUnit::rep(m_container[m_container.size() - 1])};
  }

  /**
   * @brief The elapsed time.
   */
  TUnit elapsed() const {
    return m_elapsed;
  }

  /**
   * @brief The number of increments.
   */
  std::size_t count() const {
    return m_container.size();
  }

  /**
   * @brief Get the increments.
   */
  const std::vector<double>& increments() const {
    return m_container;
  }

  /**
   * @brief The mean of the increments.
   */
  double mean() const {
    return std::accumulate(m_container.begin(), m_container.end(), 0.) / count();
  }

  /**
   * @brief The standard deviation of the increments.
   */
  double stdev() const {
    const auto m = mean();
    const auto s2 = std::inner_product(m_container.begin(), m_container.end(), m_container.begin(), 0.);
    return std::sqrt(s2 / count() - m * m);
  }

  /**
   * @brief The minimum increment.
   */
  double min() const {
    return *std::min_element(m_container.begin(), m_container.end());
  }

  /**
   * @brief The maximum increment.
   */
  double max() const {
    return *std::max_element(m_container.begin(), m_container.end());
  }

private:
  /**
   * @brief The time at which start() was called.
   */
  std::chrono::steady_clock::time_point m_tic;

  /**
   * @brief The time at which stop() was called.
   */
  std::chrono::steady_clock::time_point m_toc;

  /**
   * @brief Flag the chronometer as running (started and not stopped).
   */
  bool m_running;

  /**
   * @brief The list of increments.
   */
  std::vector<double> m_container;

  /**
   * @brief The total m_elapsed time.
   */
  TUnit m_elapsed;
};

} // namespace Litl

#endif
