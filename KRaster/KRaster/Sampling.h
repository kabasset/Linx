// Copyright (C) 2022, Antoine Basset
// This file is part of Kast.Raster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _KRASTER_SAMPLING_H
#define _KRASTER_SAMPLING_H

#include "KRaster/Region.h"

#include <vector>

namespace Kast {

/**
 * @brief Linear sampling of scalars.
 */
template <typename T = Index>
class AxisSampling {

public:
  /**
   * @brief An iterator over the sampling.
   */
  class Iterator : public std::iterator<std::input_iterator_tag, T> {

  public:
    Iterator(const AxisSampling& sampling, std::size_t index) :
        m_sampling(sampling), m_current(m_sampling.front + m_sampling.step * index), m_index(index) {}

    T operator*() {
      return m_current;
    }

    T* operator->() {
      return *m_current;
    }

    Iterator& operator++() {
      return *this += 1;
    }

    Iterator operator++(int) {
      auto res = *this;
      ++res;
    }

    Iterator& operator+=(T n) {
      m_current += m_sampling.step * n;
      m_index += n;
      return *this;
    }

    Iterator& operator-=(T n) {
      return *this += -n;
    }

    bool operator==(const Iterator& rhs) const {
      return &m_sampling == &rhs.m_sampling && m_index == rhs.m_index;
    }

    bool operator!=(const Iterator& rhs) const {
      return not(*this == rhs);
    }

  private:
    const AxisSampling<T>& m_sampling;
    T m_current;
    std::size_t m_index;
  };

  /**
   * @brief The number of samples.
   */
  std::size_t size() const {
    return std::max(T {0}, (back - front + step) / step);
  }

  /**
   * @brief Iterator to the beginning of the sampling.
   */
  Iterator begin() const {
    return {*this, 0};
  }

  /**
   * @brief Iterator to the end of the sampling.
   */
  Iterator end() const {
    return {*this, size()};
  }

  /**
   * @brief Generate a vector from the sampling.
   */
  std::vector<T> vector() const {
    return std::vector<T>(begin(), end());
  }

  /**
   * @brief The first scalar.
   */
  T front {0};

  /**
   * @brief The last scalar, possibly included (depending on `step`).
   */
  T back {-1};

  /**
   * @brief The step between two scalars.
   */
  T step {1};
};

/**
 * @brief Linear sampling of indices.
 */
using IndexSampling = AxisSampling<Index>;

/**
 * @brief Linear sampling of positions.
 */
template <Index N>
class PositionSampling {
public:
  /**
   * @brief Constructor.
   */
  PositionSampling(const Region<N>& boundingBox, const Position<N>& step = Position<N>::ones());

  /**
   * @brief Get the number of samples along each axis.
   */
  Position<N> shape() const {
    Position<N> res;
    std::transform(m_samplings.begin(), m_samplings.end(), res.begin(), [](const auto& s) {
      return s.size();
    });
    return res;
  }

  /**
   * @brief Get the sampling along a given axis.
   */
  template <Index I>
  const IndexSampling& along() const {
    return m_samplings[I];
  }

  /**
   * @copybrief along()
   */
  const IndexSampling& along(Index i) const {
    return along<i>();
  }

private:
  /**
   * @brief The 1D samplings.
   */
  std::vector<IndexSampling> m_samplings;
};

/**
 * @brief Evenly spaces elements of a contiduous container.
 */
template <typename T>
class DataSamples {

public:
  using value_type = T;

  /**
   * @brief An iterator over the samples.
   */
  template <typename TSamples>
  class Iterator : public std::iterator<std::input_iterator_tag, typename TSamples::value_type> {

  public:
    Iterator(TSamples& samples, std::size_t index) :
        m_samples(samples),
        m_it(
            m_samples.data() + m_samples.front() * m_samples.stride() + m_samples.step() * m_samples.stride() * index) {
    }

    value_type& operator*() {
      return *m_it;
    }

    value_type* operator->() {
      return m_it;
    }

    Iterator& operator++() {
      return *this += 1;
    }

    Iterator operator++(int) {
      auto res = *this;
      ++res;
    }

    Iterator& operator+=(T n) {
      m_it += m_samples.step() * m_samples.stride() * n;
      return *this;
    }

    Iterator& operator-=(T n) {
      *this += -n;
      return *this;
    }

    bool operator==(const Iterator& rhs) const {
      return m_it == rhs.m_it;
    }

    bool operator!=(const Iterator& rhs) const {
      return not(*this == rhs);
    }

    Iterator& operator=(value_type* it) {
      m_it = it;
      return *this;
    }

  private:
    TSamples& m_samples;
    value_type* m_it;
  };

  /**
   * @brief Constructor.
   */
  DataSamples(T* data, IndexSampling sampling, Index stride = 1) :
      m_data(data), m_sampling(sampling), m_stride(stride) {}

  /**
   * @brief The number of samples.
   */
  std::size_t size() const {
    return m_sampling.size();
  }

  /**
   * @brief The first sample index.
   */
  Index front() const {
    return m_sampling.front;
  }

  /**
   * @brief The last sample index.
   */
  Index back() const {
    return m_sampling.back;
  }

  /**
   * @brief The sampling step.
   */
  Index step() const {
    return m_sampling.step;
  }

  /**
   * @brief The data stride.
   */
  Index stride() const {
    return m_stride;
  }

  /**
   * @brief The data pointer.
   */
  const T* data() const {
    return m_data;
  }

  /**
   * @copybrief data()
   */
  T* data() {
    return m_data;
  }

  /**
   * @brief Increment the data pointer.
   */
  IndexSampling& operator++() {
    return *this += 1;
  }

  /**
   * @copybrief operator++()
   */
  IndexSampling operator++(int) {
    auto res = *this;
    ++res;
    return res;
  }

  /**
   * @brief Move the data pointer forward.
   */
  IndexSampling& operator+=(T n) {
    m_data += n;
    return *this;
  }

  /**
   * @brief Move the data pointer backward.
   */
  IndexSampling& operator-=(T n) {
    *this += -n;
    return *this;
  }

  /**
   * @brief Get an iterator to the beginning of the sampler.
   */
  Iterator<const DataSamples<const T>> begin() const {
    return {*this, 0};
  }

  /**
   * @brief Get an iterator to the end of the sampler.
   */
  Iterator<const DataSamples<const T>> end() const {
    return {*this, size()};
  }

  /**
   * @copybrief begin()
   */
  Iterator<DataSamples<T>> begin() {
    return {*this, 0};
  }

  /**
   * @copybrief end()
   */
  Iterator<DataSamples<T>> end() {
    return {*this, size()};
  }

private:
  /**
   * @brief The data pointer.
   */
  T* m_data;

  /**
   * @brief The index sampling.
   */
  IndexSampling m_sampling;

  /**
   * @brief The data stride.
   */
  Index m_stride;
};

} // namespace Kast

#endif // _KRASTER_SAMPLING_H
