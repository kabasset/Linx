// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_BALLITERATOR_H
#define _LITLRASTER_BALLITERATOR_H

#include "LitlRaster/Ball.h"

namespace Litl {
namespace Internal {

template <Index N, Index P>
class BallTraits<N, P>::Iterator : public std::iterator<std::input_iterator_tag, Position<N>> {

public:
  explicit Iterator(const Ball<N, P>& region, const Position<N>& current) :
      m_radiusP(std::pow(region.radius(), P)), m_center(region.center()), m_box(region.box()),
      m_current(m_box, current), m_end(m_box, endPosition(region)) {
    while (m_current != m_end && distanceToCenter() > m_radiusP) {
      ++m_current;
    }
  }

  static Position<N> beginPosition(const Ball<N, P>& region) {
    return Box<N>::Iterator::beginPosition(region.box());
  }

  static Position<N> endPosition(const Ball<N, P>& region) {
    return Box<N>::Iterator::endPosition(region.box());
  }

  const Position<N>& operator*() const {
    return *m_current;
  }

  const Position<N>* operator->() const {
    return &*m_current;
  }

  const Position<N>& operator++() {
    return next();
  }

  const Position<N>* operator++(int) {
    return &next();
  }

  bool operator==(const Iterator& rhs) const {
    return *m_current == *rhs.m_current;
  }

  bool operator!=(const Iterator& rhs) const {
    return *m_current != *rhs.m_current;
  }

private:
  inline const Position<N>& next() {
    do {
      ++m_current;
    } while (m_current != m_end && distanceToCenter() > m_radiusP);
    return *m_current;
  }

  inline double distanceToCenter() const {
    return std::inner_product(
        m_current->begin(),
        m_current->end(),
        m_center.begin(),
        0.,
        std::plus<double> {},
        [](double a, double b) {
          return std::pow(std::abs(b - a), P);
        });
  }

private:
  /**
   * @brief The ball radius to the power P.
   */
  double m_radiusP;

  /**
   * @brief The ball center.
   */
  const Position<N>& m_center;

  /**
   * @brief The bounding box.
   */
  Box<N> m_box;

  /**
   * @brief The current box iterator.
   */
  typename Box<N>::Iterator m_current;

  /**
   * @brief The ending box iterator.
   */
  typename Box<N>::Iterator m_end;
};

} // namespace Internal

/**
 * @relates Ball
 * @brief Iterator to the front position.
 */
template <Index N, Index P>
typename Ball<N, P>::Iterator begin(const Ball<N, P>& region) {
  return typename Ball<N, P>::Iterator(region, Ball<N, P>::Iterator::beginPosition(region));
}

/**
 * @relates Ball
 * @brief Iterator to one past the back position.
 */
template <Index N, Index P>
typename Ball<N, P>::Iterator end(const Ball<N, P>& region) {
  return typename Ball<N, P>::Iterator(region, Ball<N, P>::Iterator::endPosition(region));
}

} // namespace Litl

#endif
