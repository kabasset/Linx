// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXBASE_MIXINS_RANGE_H
#define _LINXBASE_MIXINS_RANGE_H

#include "Linx/Base/DataDistribution.h"

#include <algorithm>
#include <numeric> // accumulate

namespace Linx {

/**
 * @ingroup mixins
 * @brief Base class to provide range operations.
 * @tparam T The value type
 * @tparam TDerived The child class which implements required methods
 */
template <typename T, typename TDerived>
struct RangeMixin {
  /// @{
  /// @group_modifiers

  /**
   * @brief Fill the container with a single value.
   */
  TDerived& fill(const T& value)
  {
    std::fill(static_cast<TDerived&>(*this).begin(), static_cast<TDerived&>(*this).end(), value);
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief Fill the container with evenly spaced value.
   * 
   * The difference between two adjacent values is _exactly_ `step`,
   * i.e. `container[i + 1] = container[i] + step`.
   * This means that rounding errors may sum up,
   * as opposed to `linspace()`.
   * @see `linspace()`
   */
  TDerived& range(const T& min = Limits<T>::zero(), const T& step = Limits<T>::one())
  {
    auto v = min;
    auto& t = static_cast<TDerived&>(*this);
    for (auto& e : t) {
      e = v;
      v += step;
    }
    return t;
  }

  /**
   * @brief Fill the container with evenly spaced value.
   * 
   * The first and last values of the container are _exactly_ `min` and `max`.
   * Intermediate values are computed as `container[i] = min + (max - min) / (size() - 1) * i`,
   * which means that the difference between two adjacent values
   * is not necessarily perfectly constant for floating point values,
   * as opposed to `range()`.
   * @see `range()`
   */
  TDerived& linspace(const T& min = Limits<T>::zero(), const T& max = Limits<T>::one())
  {
    const std::size_t size = std::distance(static_cast<TDerived&>(*this).begin(), static_cast<TDerived&>(*this).end());
    const auto step = (max - min) / (size - 1);
    auto it = static_cast<TDerived&>(*this).begin();
    for (std::size_t i = 0; i < size - 1; ++i, ++it) {
      *it = min + step * i;
    }
    *it = max;
    return static_cast<TDerived&>(*this);
  }

  /**
   * @brief Generate values from a function with optional input containers.
   * @param func The generator function, which takes as many inputs as there are arguments
   * @param args The arguments in the form of containers of compatible sizes
   * 
   * For example, here is how to imlement element-wise square root and multiplication:
   * \code
   * Container a = ...;
   * Container b = ...;
   * Container res(a.size());
   * res.generate([](auto v) { return std::sqrt(v); }, a); // res = sqrt(a)
   * res.generate([](auto v, auto w) { return v * w; }, a, b); // res = a * b
   * \endcode
   */
  template <typename TFunc, typename... TContainers>
  TDerived& generate(TFunc&& func, const TContainers&... args)
  {
    auto its = std::make_tuple(args.begin()...);
    auto& t = static_cast<TDerived&>(*this);
    for (auto& v : t) {
      v = iterator_tuple_apply(its, func);
    }
    return t;
  }

  /**
   * @brief Apply a function with optional input containers.
   * @param func The function
   * @param args The arguments in the form of containers of compatible sizes
   * 
   * If there are _n_ arguments, `func` takes _n_+1 parameters,
   * where the first argument is the element of this container.
   * For example, here is how to imlement in-place element-wise square root and multiplication:
   * \code
   * Container a = ...;
   * Container res = ...;
   * res.apply([](auto v) { return std::sqrt(v); }); // res = sqrt(res)
   * res.apply([](auto v, auto w) { return v * w; }, a); // res *= a
   * \endcode
   */
  template <typename TFunc, typename... TContainers>
  TDerived& apply(TFunc&& func, const TContainers&... args)
  {
    return generate(std::forward<TFunc>(func), static_cast<TDerived&>(*this), args...);
  }

  /**
   * @brief Reverse the order of the elements.
   */
  TDerived& reverse()
  {
    auto& t = static_cast<TDerived&>(*this);
    std::reverse(t.begin(), t.end());
    return t;
  }

  /// @group_operations

  /**
   * @brief Get a reference to the (first) min element.
   * @see `distribution()`
   */
  const T& min() const
  {
    return *std::min_element(static_cast<const TDerived&>(*this).begin(), static_cast<const TDerived&>(*this).end());
  }

  /**
   * @brief Get a reference to the (first) max element.
   * @see `distribution()`
   */
  const T& max() const
  {
    return *std::max_element(static_cast<const TDerived&>(*this).begin(), static_cast<const TDerived&>(*this).end());
  }

  /**
   * @brief Get a pair of references to the (first) min and max elements.
   * @see `distribution()`
   */
  std::pair<const T&, const T&> minmax() const
  {
    const auto its =
        std::minmax_element(static_cast<const TDerived&>(*this).begin(), static_cast<const TDerived&>(*this).end());
    return {*its.first, *its.second};
  }

  /**
   * @brief Create a `DataDistribution` from the container.
   */
  [[deprecated("Use free function")]] DataDistribution<T> distribution() const
  {
    return DataDistribution<T>(static_cast<const TDerived&>(*this));
  }

  /// @}
};

/**
 * @relatesalo RangeMixin
 * @brief Get a reference to the (first) min element.
 * @see `distribution()`
 */
template <typename TRange>
const typename TRange::value_type& min(const TRange& in)
{
  return *std::min_element(in.begin(), in.end());
}

/**
 * @relatesalo RangeMixin
 * @brief Get a reference to the (first) max element.
 * @see `distribution()`
 */
template <typename TRange>
const typename TRange::value_type& max(const TRange& in)
{
  return *std::max_element(in.begin(), in.end());
}

/**
 * @relatesalo RangeMixin
 * @brief Get a pair of references to the (first) min and max elements.
 * @see `distribution()`
 */
template <typename TRange>
std::pair<const typename TRange::value_type&, const typename TRange::value_type&> minmax(const TRange& in)
{
  const auto its = std::minmax_element(in.begin(), in.end());
  return {*its.first, *its.second};
}

/**
 * @relatesalo RangeMixin
 * @brief Compute the sum of a range.
 * @param offset An offset
 */
template <typename TRange>
double sum(const TRange& in, double offset = 0)
{
  return std::accumulate(in.begin(), in.end(), offset);
}

/**
 * @relatesalo RangeMixin
 * @brief Compute the product of a range.
 * @param factor A factor
 */
template <typename TRange>
double product(const TRange& in, double factor = 1)
{
  return std::accumulate(in.begin(), in.end(), factor, std::multiplies<double> {});
}

/**
 * @relatesalo RangeMixin
 * @brief Compute the mean of a range.
 * @see `distribution()`
 */
template <typename TRange>
double mean(const TRange& in)
{
  return sum(in) / in.size();
}

/**
 * @relatesalo RangeMixin
 * @brief Create a `DataDistribution` from the container.
 */
template <typename TRange>
DataDistribution<typename TRange::value_type> distribution(const TRange& in)
{
  return DataDistribution<typename TRange::value_type>(in);
}

} // namespace Linx

#endif
