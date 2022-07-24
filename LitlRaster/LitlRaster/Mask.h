/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLRASTER_MASK_H
#define _LITLRASTER_MASK_H

#include "LitlContainer/Sequence.h"
#include "LitlRaster/Box.h"

namespace Litl {

/**
 * @brief A masked ND bounding box.
 * @details
 * This class is similar to `Box`, yet with a boolean value (the flag) associated to each position.
 */
template <Index N = 2>
class Mask : boost::additive<Mask<N>, Position<N>>, boost::additive<Mask<N>, Index> {

public:
  /**
   * @brief A position iterator.
   * @see `Box::Iterator`
   */
  class Iterator;

  /// @{
  /// @group_construction

  /**
   * @brief Constructor.
   */
  explicit Mask(Position<N> front, Position<N> back) :
      m_box(std::move(front), std::move(back)), m_flags(m_box.size(), true) {}

  /**
   * @brief Create a mask from a radius and center position.
   */
  static Mask<N> fromCenter(Index radius = 1, const Position<N> center = Position<N>::zero()) {
    return Mask<N>(center - radius, center + radius);
  }

  /**
   * @brief Create a mask from a ball with (pseudo-)norm L0, L1 or L2.
   * @tparam P The norm power
   */
  template <Index P>
  static Mask<N> ball(double radius = 1) {
    auto out = Mask<N>::fromCenter(radius);
    const auto radiusPow = std::pow(radius, P);
    auto it = out.m_flags.begin();
    for (const auto& p : out.box()) {
      if (p.template norm<P>() <= radiusPow) { // TODO optimize for 0?
        *it = true;
      }
      ++it;
    }
    return out;
  }

  /// @group_properties

  /**
   * @brief Get the bounding box.
   */
  const Box<N>& box() const {
    return m_box;
  }

  /**
   * @brief Compute the box shape.
   */
  Position<N> shape() const {
    return m_box.shape();
  }

  /**
   * @brief Get the number of dimensions.
   */
  Index dimension() const {
    return m_box.dimension();
  }

  /**
   * @brief Compute the mask size, i.e. number of positions.
   */
  Index size() const {
    return std::count(m_flags.begin(), m_flags.end(), true);
  }

  /**
   * @brief Get the bounding box length along given axis.
   */
  template <Index I>
  Index length() const {
    return m_box.template length<I>();
  }

  /**
   * @brief Get the bounding box length along given axis.
   */
  Index length(Index i) const {
    return m_box.length(i);
  }

  /// @group_operations

  /**
   * @brief Check whether two masks are equal.
   */
  bool operator==(const Mask<N>& other) const {
    return m_box == other.m_box && m_flags == other.m_flags;
  }

  /**
   * @brief Check whether two masks are different.
   */
  bool operator!=(const Mask<N>& other) const {
    return m_box != other.m_box || m_flags != other.m_flags;
  }

  /**
   * @brief Get the row-major ordered list of positions in the mask.
   */
  Sequence<Position<N>> domain() const {
    std::vector<Position<N>> vec;
    auto it = m_flags.begin();
    for (const auto& p : m_box) {
      if (*it) {
        vec.push_back(p);
      }
      ++it;
    }
    return Sequence<Position<N>>(vec.size(), std::move(vec)); // FIXME size() invalid if move is made beforehand
  }

  /// @group_modifiers

  /**
   * @brief Shift the mask by a given vector.
   */
  Mask<N>& operator+=(const Position<N>& shift) {
    m_box += shift;
    return *this;
  }

  /**
   * @brief Shift the mask by a given vector.
   */
  Mask<N>& operator-=(const Position<N>& shift) {
    m_box -= shift;
    return *this;
  }

  /**
    * @brief Add a scalar to each coordinate.
    */
  Mask<N>& operator+=(Index scalar) {
    m_box += scalar;
    return *this;
  }

  /**
   * @brief Subtract a scalar to each coordinate.
   */
  Mask<N>& operator-=(Index scalar) {
    m_box -= scalar;
    return *this;
  }

  /**
   * @brief Add 1 to each coordinate.
   */
  Mask<N>& operator++() {
    return *this += 1;
  }

  /**
   * @brief Subtract 1 to each coordinate.
   */
  Mask<N>& operator--() {
    return *this -= 1;
  }

  /**
   * @brief Copy.
   */
  Mask<N> operator+() {
    return *this;
  }

  /**
   * @brief Invert the sign of each coordinate.
   */
  Mask<N> operator-() {
    auto out = *this;
    out.m_box = -m_box;
    std::reverse(out.m_flags.begin(), out.m_flags.end());
    return out; // FIXME optimize
  }

  /// @}

private:
  /**
   * @brief The bounding box.
   */
  Box<N> m_box;

  /**
   * @brief The flag map.
   */
  std::vector<bool> m_flags;
};

} // namespace Litl

#include "LitlRaster/MaskIterator.h"

#endif
