// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_DATADISTRIBUTION_H
#define _RASTER_DATADISTRIBUTION_H

#include "RasterTypes/TypeUtils.h"

#include <algorithm>
#include <vector>

namespace Cnes {

/**
 * @ingroup data_classes
 * @brief Estimate data distribution parameters of a container.
 * @tparam T The element type
 * @details
 * The input container values are copied inside the class at construction,
 * and there is no link between the container and the `DataDistribution` after that,
 * such that eventual changes to the input container would not be reflected.
 * 
 * Most estimators rely on partially sorted values.
 * The class relies on lazy evaluation to perform just enough sorting to output the requested parameters.
 * Sequentially calling several methods results in a more and more sorted values.
 * If a lot of different parameters have to be estimated,
 * then it might be faster to completely sort the values by calling `sort()` beforehand.
 */
template <typename T>
class DataDistribution {

public:
  using Floating = typename TypeTraits<T>::Floating;

  /// @{
  /// @group_construction

  /**
   * @brief Size-based constructor.
   */
  template <typename TIterable>
  explicit DataDistribution(const TIterable& values) :
      m_values(values.begin(), values.end()), m_sorted(std::is_sorted(m_values.begin(), m_values.end())) {}

  /// @group_properties

  std::size_t size() const {
    return m_values.size();
  }

  /// @group_operators

  /**
   * @brief Sort the values once for all.
   */
  void sort() {
    if (not m_sorted) {
      std::sort(m_values.begin(), m_values.end());
      m_sorted = true;
    }
  }

  /**
   * @brief Get a reference to the (first) n-th smallest element.
   */
  const T& nth(std::size_t n) {
    if (m_sorted) {
      return m_values[n];
    }
    auto it = m_values.begin() + n;
    std::nth_element(m_values.begin(), it, m_values.end());
    return *it;
  }

  /**
   * @brief Get the q-th quantile (with linear interpolation).
   * @details
   * The following values of `q` correspond to equivalent functions:
   * - 0: `min()`;
   * - 1: `max()`;
   * - 0.5: `median()`.
   */
  Floating quantile(double q) {
    const auto n = q * (size() - 1);
    const std::size_t f = n;
    if (n == f) {
      return nth(f);
    }
    const auto d = n - f;
    return nth(f) * d + nth(f + 1) * (1. - d); // FIXME linear(&nth(f), d);
  }

  /**
   * @brief Get the median.
   */
  Floating median() {
    return quantile(0.5);
  }

  /**
   * @brief Get a reference to the min element.
   */
  const T& min() {
    return nth(0);
  }

  /**
   * @brief Get a reference to the max element.
   */
  const T& max() {
    return nth(size() - 1);
  }

  /**
   * @brief Compute the histogram with given bins.
   * @details
   * The output size is the size of `bins` minus one.
   */
  template <typename TIterable>
  std::vector<std::size_t> histogram(const TIterable& bins) { // FIXME bounds options
    sort();
    std::vector<std::size_t> out(std::distance(bins.begin(), bins.end()) - 1);
    auto it = m_values.begin();
    auto supIt = bins.begin();
    auto countIt = out.begin();
    while (it != m_values.end() && *it < *supIt) {
      ++it;
    }
    for (++supIt; supIt != bins.end() && countIt != out.end(); ++supIt) {
      while (it != m_values.end() && *it < *supIt) {
        ++(*countIt);
        ++it;
      }
      ++countIt;
    }
    if (it != m_values.end() && *it == bins.back()) {
      ++(*countIt);
    }
    return out;
  }

  /// @}

private:
  /**
   * @brief The partially sorted values.
   */
  std::vector<T> m_values;

  /**
   * @brief Check if values are totally sorted.
   */
  bool m_sorted;
};

} // namespace Cnes

#endif
