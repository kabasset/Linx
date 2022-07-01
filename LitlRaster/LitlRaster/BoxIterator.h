// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_BOXITERATOR_H
#define _LITLRASTER_BOXITERATOR_H

#include "LitlRaster/Box.h"

namespace Litl {

template <Index N>
class Box<N>::Iterator : public std::iterator<std::input_iterator_tag, Position<N>> {

public:
  /**
   * @brief Constructor.
   * @param region The box to be screened
   * @param followers Positions which follow the same moves as the current position
   */
  explicit Iterator(const Box<N>& region, const std::vector<Position<N>>& followers = {}) :
      m_region(region), m_current(region.front()), m_fronts(followers), m_followers(followers) {}

  /**
   * @brief Dereference operator.
   */
  const Position<N>& operator*() const {
    return m_current;
  }

  /**
   * @brief Arrow operator.
   */
  const Position<N>* operator->() const {
    return &m_current;
  }

  /**
   * @brief Increment operator.
   */
  const Position<N>& operator++() {
    next();
    return m_current;
  }

  /**
   * @brief Increment operator.
   */
  const Position<N>* operator++(int) {
    next();
    return &m_current;
  }

  /**
   * @brief Equality operator.
   */
  bool operator==(const Iterator& rhs) const {
    return m_current == rhs.m_current;
  }

  /**
   * @brief Non-equality operator.
   */
  bool operator!=(const Iterator& rhs) const {
    return m_current != rhs.m_current;
  }

  /**
   * @brief Get the followers positions.
   */
  const std::vector<Position<N>>& followers() const {
    return m_followers;
  }

  /**
   * @brief Reset the current and followers positions to the initial positions.
   */
  void reset() {
    m_current = m_region.front();
    m_followers = m_fronts;
  }

private:
  /**
   * @brief Update and get the current position and that of a follower.
   * @details
   * Move the current position by 1 pixel,
   * such that the corresponding index in a `Raster` would be increased to the next one.
   * 
   * Conventionally, `next(region.back())[i] = region.back()[i]` for `i > 0`,
   * and `next(region.back())[0] = region.back()[0] + 1`.
   */
  inline const Position<N>& next() {
    if (m_current == m_region.back()) {
      m_current[0]++;
      return m_current;
    }
    m_current[0]++;
    for (auto& f : m_followers) {
      f[0]++;
    }
    for (std::size_t i = 0; i < m_current.size(); ++i) {
      if (m_current[i] > m_region.back()[i]) {
        m_current[i] = m_region.front()[i];
        m_current[i + 1]++;
        for (std::size_t j = 0; j < m_followers.size(); ++j) {
          m_followers[j][i] = m_fronts[j][i];
          m_followers[j][i + 1]++;
        }
      }
    }
    return m_current;
  }

private:
  /**
   * @brief The screened region.
   */
  const Box<N>& m_region;

  /**
   * @brief The current position.
   */
  Position<N> m_current;

  /**
   * @brief The front positions of the followers.
   */
  std::vector<Position<N>> m_fronts; // FIXME keep?

  /**
   * @brief The current position of the followers.
   */
  std::vector<Position<N>> m_followers;
};

/**
 * @brief Iterator to the front position.
 */
template <Index N>
typename Box<N>::Iterator begin(const Box<N>& box) {
  return typename Box<N>::Iterator(box);
}

/**
 * @brief Iterator to one past the back position.
 */
template <Index N>
typename Box<N>::Iterator end(const Box<N>& box) {
  auto front = box.back();
  ++front[0];
  return typename Box<N>::Iterator({front, box.back()});
}

} // namespace Litl

#endif
