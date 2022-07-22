/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_BALL_H
#define _LITLRASTER_BALL_H

#include "LitlContainer/Sequence.h"
#include "LitlRaster/Box.h"

namespace Litl {

/// @cond
namespace Internal {

template <Index N, Index P>
struct BallTraits {
  struct Iterator;
};

template <Index N>
struct BallTraits<N, 0> {
  struct Iterator;
};

} // namespace Internal
/// @endcond

/**
 * @brief An N-ball region with (pseudo-)norm L0, L1 or L2.
 * @tparam N The dimension (must be positive)
 * @tparam P The norm (> 0) or pseudo-norm (= 0)
 * @details
 * The ball radius is a floating point number.
 * The boundary of the ball is included.
 * 
 * The N-ball with L0-pseudo-norm is a cross.
 * The N-ball with L1-norm corresponds to the cross-polytope of dimension N.
 * The N-ball with L∞-norm would simply be `Box<N>`.
 */
template <Index N = 2, Index P = 2>
class Ball : boost::additive<Ball<N, P>, Position<N>>, boost::additive<Ball<N, P>, Index> {

public:
  /**
   * @brief A position iterator.
   * @see `Box::Iterator`
   */
  using Iterator = typename Internal::BallTraits<N, P>::Iterator;

  /// @{
  /// @group_construction

  /**
   * @brief Create a ball from a radius and center position.
   */
  explicit Ball(double radius, Position<N> center = Position<N>::zero()) :
      m_radius(radius), m_center(std::move(center)) {}

  /// @group_properties

  /**
   * @brief Get the radius.
   */
  double radius() const {
    return m_radius;
  }

  /**
   * @brief Get the center.
   */
  const Position<N>& center() const {
    return m_center;
  }

  /**
   * @brief Compute the bounding box.
   * @details
   * Note that the bounding box's radius is less than the ball radius
   * when the latter is not an integer.
   */
  Box<N> box() const {
    return Box<N>::fromCenter(m_radius, m_center);
  }

  /**
   * @brief Compute the bounding box shape.
   */
  Position<N> shape() const {
    return Position<N>(N).fill(length<0>());
  }

  /**
   * @brief Get the number of dimensions.
   */
  Index dimension() const {
    return m_center.size();
  }

  /**
   * @brief Compute the ball size, i.e. number of positions.
   */
  Index size() const {
    return std::distance(begin(*this), end(*this));
  }

  /**
   * @brief Get the bounding box length along given axis.
   */
  template <Index = 0>
  Index length() const {
    return 2 * Index(m_radius) + 1;
  }

  /**
   * @brief Get the bounding box length along given axis.
   */
  Index length(Index = 0) const {
    return 2 * Index(m_radius) + 1;
  }

  /// @group_operations

  /**
   * @brief Check whether two balls are equal.
   */
  bool operator==(const Ball<N, P>& other) const {
    return m_radius == other.m_radius && m_center == other.m_center;
  }

  /**
   * @brief Check whether two balls are different.
   */
  bool operator!=(const Ball<N, P>& other) const {
    return m_radius != other.m_radius || m_center != other.m_center;
  }

  /**
   * @brief Get the row-major ordered sequence of positions inside the ball.
   */
  Sequence<Position<N>> domain() const {
    std::vector<Position<N>> out; // TODO reserve the ball's volume?
    for (const auto& p : *this) {
      out.push_back(p);
    }
    return Sequence<Position<N>>(out.size(), std::move(out));
  }

  /// @group_modifiers

  /**
   * @brief Shift the ball by a given vector.
   */
  Ball<N, P>& operator+=(const Position<N>& shift) {
    m_center += shift;
    return *this;
  }

  /**
   * @brief Shift the ball by a given vector.
   */
  Ball<N, P>& operator-=(const Position<N>& shift) {
    m_center -= shift;
    return *this;
  }

  /**
    * @brief Add a scalar to each coordinate.
    */
  Ball<N, P>& operator+=(Index scalar) {
    m_center += scalar;
    return *this;
  }

  /**
   * @brief Subtract a scalar to each coordinate.
   */
  Ball<N, P>& operator-=(Index scalar) {
    m_center -= scalar;
    return *this;
  }

  /**
   * @brief Add 1 to each coordinate.
   */
  Ball<N, P>& operator++() {
    return *this += 1;
  }

  /**
   * @brief Subtract 1 to each coordinate.
   */
  Ball<N, P>& operator--() {
    return *this -= 1;
  }

  /**
   * @brief Copy.
   */
  Ball<N, P> operator+() {
    return *this;
  }

  /**
   * @brief Invert the sign of each center coordinate.
   */
  Ball<N, P> operator-() {
    return Ball<N, P>(m_radius, -m_center);
  }

  /// @}

private:
  /**
   * @brief The radius.
   */
  double m_radius;

  /**
   * @brief The center.
   */
  Position<N> m_center;
};

} // namespace Litl

#include "LitlRaster/BallIterator.h"

#endif
