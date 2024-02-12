// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXRUN_TIMER_H
#define _LINXRUN_TIMER_H

#include "Linx/Base/DataDistribution.h"

#include <algorithm> // min_element, max_element
#include <chrono>
#include <vector>

namespace Linx {

/**
 * @brief A simple timer with split times and elapsed time recording.
 * @tparam TUnit The time unit, e.g. `std::chrono::milliseconds`
 * 
 * Each time the timer is started and stopped, a split is computed,
 * and the total elapsed time is incremented.
 * Split times can also be requested without stopping the timer.
 * An offset can be provided, which is the initial value of the elapsed time,
 * but has no effect on the split times.
 *
 * Simple statistics on the split times can be computed (e.g. mean or standard deviation).
 *
 * The timer can be reset, which means that the list of split times is emptied,
 * and the elapsed time is set to 0 or a given offset.
 */
template <typename TUnit>
class Timer {
public:

  /**
   * @brief The time unit.
   */
  using Unit = TUnit;

  /**
   * @brief Create a timer with optional offset.
   */
  explicit Timer(TUnit offset = TUnit()) : m_tic(), m_toc(), m_running(false), m_container(), m_elapsed(offset)
  {
    reset(offset);
  }

  /**
   * @brief Reset the timer with optional offset.
   */
  void reset(TUnit offset = TUnit())
  {
    m_toc = m_tic;
    m_running = false;
    m_container.resize(0);
    m_elapsed = offset;
  }

  /**
   * @brief Start or restart the timer.
   */
  void start()
  {
    m_tic = std::chrono::steady_clock::now();
    m_running = true;
  }

  /**
   * @brief Stop the timer and get the last split time.
   */
  TUnit stop()
  {
    m_toc = std::chrono::steady_clock::now();
    m_running = false;
    const auto inc = std::chrono::duration_cast<TUnit>(m_toc - m_tic);
    m_elapsed += inc;
    m_container.push_back(inc.count());
    return inc;
  }

  /**
   * @brief Get a split time without stopping the timer.
   * 
   * The split is recorded, such that the function is conceptually equivalent to calling `stop()` and `start()` in a row,
   * but does not introduce any latency.
   */
  TUnit split()
  {
    m_toc = std::chrono::steady_clock::now();
    const auto inc = std::chrono::duration_cast<TUnit>(m_toc - m_tic);
    m_elapsed += inc;
    m_container.push_back(inc.count());
    m_tic = m_toc;
    return inc;
  }

  /**
   * @brief Test whether the timer is running.
   */
  bool is_running() const
  {
    return m_running; // TODO m_running could be removed by comparing m_toc to m_tic, but is it relevant?
  }

  /**
   * @brief Get the i-th split time.
   */
  TUnit operator[](std::size_t i) const
  {
    return TUnit {typename TUnit::rep(m_container[i])};
  }

  /**
   * @brief Get the first split time.
   */
  TUnit front() const
  {
    return operator[](0);
  }

  /**
   * @brief Get the last split time.
   */
  TUnit back() const
  {
    return operator[](m_container.size() - 1);
  }

  /**
   * @brief Get the total elapsed time.
   */
  TUnit total() const // FIXME rename as sum?
  {
    return m_elapsed;
  }

  /**
   * @brief Get the number of split times.
   */
  std::size_t size() const
  {
    return m_container.size();
  }

  /**
   * @brief Get the split times as `double`s.
   */
  const std::vector<double>& container() const
  {
    return m_container;
  }

  /**
   * @brief Get the minimum split time.
   * @see `distribution()`
   */
  double min() const
  {
    return *std::min_element(m_container.begin(), m_container.end());
  }

  /**
   * @brief Get the maximum split time.
   * @see `distribution()`
   */
  double max() const
  {
    return *std::max_element(m_container.begin(), m_container.end());
  }

  /**
   * @brief Get the pair of min and max split times.
   * @see `distribution()`
   */
  std::pair<TUnit, TUnit> minmax() const
  {
    const auto its = std::minmax_element(m_container.begin(), m_container.end());
    return {*its.first, *its.second};
  }

  /**
   * @brief Get the split times distribution.
   */
  DataDistribution<double> distribution() const
  {
    return DataDistribution<double>(m_container);
  }

private:

  /**
   * @brief The time at which `start()` was called.
   */
  std::chrono::steady_clock::time_point m_tic;

  /**
   * @brief The time at which `stop()` was called.
   */
  std::chrono::steady_clock::time_point m_toc;

  /**
   * @brief Flag the timer as running (started and not stopped).
   */
  bool m_running;

  /**
   * @brief The list of split times.
   */
  std::vector<double> m_container;

  /**
   * @brief The total elapsed time.
   */
  TUnit m_elapsed;
};

} // namespace Linx

#endif
