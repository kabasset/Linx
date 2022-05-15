// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_STATS_H
#define _RASTER_STATS_H

#include <algorithm>
#include <cmath>
#include <vector>

namespace Cnes {

enum StatsMode
{
  Standard, ///< Use standard estimators
  Unbiased, ///< Use unbiased standard deviation
  Robust ///< Use median and MAD
};

template <StatsMode Mode>
struct Stats {
  template <typename TContainer>
  Stats(const TContainer& container);
  std::size_t count;
  double min;
  double max;
  double mean;
  double deviation;
};

template <typename T>
class SortedVector {

public:
  template <typename TContainer>
  SortedVector(TContainer&& container) : m_container(std::move(container)) {
    std::sort(m_container.begin(), m_container.end());
  }

  template <typename TContainer>
  SortedVector& operator=(TContainer&& container) {
    m_container = std::move(container);
    std::sort(m_container.begin(), m_container.end());
    return *this;
  }

  void moveTo(std::vector<T>& out) {
    out = std::move(m_container);
  }

  const T& min() const {
    return m_container[0];
  }

  const T& max() const {
    return m_container[m_container.size() - 1];
  }

  template <typename TContainer>
  const T& kthSmallest(std::size_t k) const {
    return m_container[k - 1];
  }

  double median() {
    return kthQuantile(.5);
  }

  double kthQuantile(double k) {
    const auto size = m_container.size();
    const double kDouble = k * (size - 1);
    const std::size_t kInt = kDouble;
    if (kDouble == kInt) {
      return m_container[kInt];
    }
    const double d = kDouble - kInt;
    return d * m_container[kInt] + (1 - d) * m_container[kInt + 1]; // linear interpolation
  }

  std::vector<std::size_t> histogram(const std::vector<T>& bins) {
    std::vector<std::size_t> out(bins.size() - 1);
    for (const auto e : m_container) {
      // FIXME
    }
    return out;
  }

private:
  std::vector<T> m_container;
};

template <>
template <typename TContainer>
Stats<StatsMode::Standard>::Stats(const TContainer& container) :
    count(container.size()), min(container[0]), max(min), mean(0), deviation(0) {
  double sum = 0;
  for (const auto& e : container) {
    if (e < min) {
      min = e;
    } else if (e > max) {
      max = e;
    };
    sum += e;
    deviation += e * e;
  }
  mean = sum / count;
  deviation = std::sqrt((deviation - sum * mean) / count);
}

template <>
template <typename TContainer>
Stats<StatsMode::Unbiased>::Stats(const TContainer& container) :
    count(container.size()), min(container[0]), max(min), mean(0), deviation(0) {
  double sum = 0;
  for (const auto& e : container) {
    if (e < min) {
      min = e;
    } else if (e > max) {
      max = e;
    };
    sum += e;
    deviation += e * e;
  }
  mean = sum / count;
  deviation = std::sqrt((deviation - sum * mean) / (count - 1));
}

template <>
template <typename TContainer>
Stats<StatsMode::Robust>::Stats(const TContainer& container) :
    count(container.size()), min(0), max(min), mean(0), deviation(0) {
  SortedVector<typename TContainer::value_type> sorted(container);
  min = sorted.min();
  max = sorted.max();
  mean = sorted.median();
  std::vector<typename TContainer::value_type> absdev(count);
  sorted.moveTo(absdev);
  for (auto it = absdev.begin(); it != absdev.end(); ++it) {
    *it = std::abs(*it - mean);
  }
  sorted = absdev;
  deviation = sorted.median();
}

} // namespace Cnes

#endif // _RASTER_STATS_H
