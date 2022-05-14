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
  Unbiased, ///< Use unbiased variance
  Robust ///< Use median and MAD
};

class Stats {
  StatsMode mode;
  double min;
  double max;
  double mean;
  double variance;
};

template <typename T>
class SortedVector {

public:
  template <typename TContainer>
  SortedVector(TContainer&& container) : m_container(std::move(container)) {
    std::sort(m_container.begin(), m_container.end());
  }

  template <typename TContainer>
  const typename TContainer::value_type& kthSmallest(std::size_t k) {
    return m_container[k - 1];
  }

  double median() {
    return kthQuantile(.5);
  }

  double kthQuantile(double k) {
    const auto size = m_container.size();
    const double kDouble = k * (size + 1);
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

} // namespace Cnes

#endif // _RASTER_STATS_H
