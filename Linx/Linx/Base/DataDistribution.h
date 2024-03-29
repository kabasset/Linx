// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXBASE_DATADISTRIBUTION_H
#define _LINXBASE_DATADISTRIBUTION_H

#include "Linx/Base/TypeUtils.h"

#include <algorithm>
#include <vector>

namespace Linx {

/**
 * @ingroup data_classes
 * @brief Estimate data distribution parameters of a container.
 * @tparam T The element type
 * 
 * The input container values are copied inside the class at construction,
 * and there is no link between the container and the `DataDistribution` after that,
 * such that eventual changes to the input container would not be reflected.
 * 
 * Most estimators rely on partially sorted values.
 * The class performs evaluation to sort the values just enough to return the requested parameters.
 * Sequentially calling several methods results in a more and more sorted values.
 * If a lot of different parameters have to be estimated,
 * then it might be faster to completely sort the values by calling `sort()` beforehand.
 * 
 * Methods are not `const` because they involve sorting or caching.
 */
template <typename T>
class DataDistribution {
public:

  /**
   * @copybrief TypeTraits::Floating
   */
  using Floating = typename TypeTraits<T>::Floating;

  /// @{
  /// @group_construction

  /**
   * @brief Vector-move constructor.
   */
  explicit DataDistribution(std::vector<T>&& values) :
      m_values(std::move(values)), m_sorted(std::is_sorted(m_values.begin(), m_values.end())), m_sum(Limits<T>::zero()),
      m_sum2(m_sum)
  {
    for (const auto& v : m_values) {
      m_sum += v;
      m_sum2 += v * v;
    }
  }

  /**
   * @brief Range-copy constructor.
   */
  template <typename TRange>
  explicit DataDistribution(const TRange& values) : DataDistribution(std::vector<T>(values.begin(), values.end()))
  {}

  /// @group_properties

  /**
   * @brief Get the number of values.
   */
  std::size_t size()
  {
    return m_values.size();
  }

  /**
   * @brief Get a reference to the min element.
   */
  const T& min()
  {
    return nth(0);
  }

  /**
   * @brief Get a reference to the max element.
   */
  const T& max()
  {
    return nth(size() - 1);
  }

  /**
   * @brief Get the sum of all values.
   */
  const T& sum()
  {
    return m_sum;
  }

  /**
   * @brief Get a reference to the n-th smallest element.
   */
  const T& nth(std::size_t n)
  {
    if (m_sorted) {
      return m_values[n];
    }
    auto it = m_values.begin() + n;
    std::nth_element(m_values.begin(), it, m_values.end());
    return *it;
  }

  /// @group_operations

  /**
   * @brief Compute the mean.
   */
  Floating mean()
  {
    return sum() / size();
  }

  /**
   * @brief Compute the median.
   */
  Floating median()
  {
    return quantile(0.5);
  }

  /**
   * @brief Compute the variance.
   * 
   * The difference between biased and unbiased variance is that the denomitator is
   * the number of samples `size()` in the first case
   * and `size() - 1` in the latter case.
   */
  Floating variance(bool unbiased = true)
  {
    return Floating(m_sum2 - m_sum * m_sum / size()) / (size() - unbiased);
  }

  /**
   * @brief Compute the standard deviation.
   */
  Floating stdev(bool unbiased = true)
  {
    return std::sqrt(variance(unbiased));
  }

  /**
   * @brief Compute the median absolute deviation.
   */
  Floating mad()
  {
    std::vector<T> absdev(size());
    const auto m = median();
    std::transform(m_values.begin(), m_values.end(), absdev.begin(), [=](auto e) {
      return std::abs(e - m);
    });
    return DataDistribution<T>(std::move(absdev)).median();
  }

  /**
   * @brief Compute the q-th quantile (with linear interpolation).
   * 
   * The following values of `q` correspond to equivalent functions:
   * - 0: `min()`;
   * - 1: `max()`;
   * - 0.5: `median()`.
   */
  Floating quantile(double q)
  {
    const auto n = q * (size() - 1);
    const std::size_t f = n;
    if (n == f) {
      return nth(f);
    }
    const auto d = n - f;
    return nth(f) * d + nth(f + 1) * (1. - d); // FIXME linear(&nth(f), d);
  }

  /**
   * @brief Compute the histogram with given bins.
   * 
   * The output size is the size of `bins` minus one.
   * Bins are open-closed intervals (the lower bound is includer, the upper bound is excluded).
   */
  template <typename TRange>
  std::vector<std::size_t> histogram(const TRange& bins)
  { // FIXME bounds options
    sort();
    std::vector<std::size_t> out(std::distance(bins.begin(), bins.end()) - 1);
    auto it = m_values.begin();
    auto sup_it = bins.begin();
    auto count_it = out.begin();
    while (it != m_values.end() && *it < *sup_it) {
      ++it;
    }
    for (++sup_it; sup_it != bins.end() && count_it != out.end(); ++sup_it) {
      while (it != m_values.end() && *it < *sup_it) {
        ++(*count_it);
        ++it;
      }
      ++count_it;
    }
    if (it != m_values.end() && *it == bins.back()) {
      ++(*count_it);
    }
    return out;
  }

  /**
   * @brief Sort the values once for all.
   */
  void sort()
  {
    if (not m_sorted) {
      std::sort(m_values.begin(), m_values.end());
      m_sorted = true;
    }
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
  bool m_sorted; // FIXME count the number of elements sorted instead

  /**
   * @brief The cached sum.
   */
  T m_sum;

  /**
   * @brief The cached sum of squares.
   */
  T m_sum2;
};

} // namespace Linx

#endif
