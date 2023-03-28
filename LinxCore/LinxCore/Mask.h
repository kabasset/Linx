/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXCORE_MASK_H
#define _LINXCORE_MASK_H

#include "LinxCore/Box.h"
#include "LinxCore/Raster.h"
#include "LinxCore/Sequence.h"

namespace Linx {

/**
 * @ingroup regions
 * @brief A masked ND bounding box.
 * 
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
  explicit Mask(Box<N> box, bool flag = true) : m_box(std::move(box)), m_flags(m_box.shape()) {
    m_flags.fill(flag);
  }

  /**
   * @brief Constructor.
   */
  explicit Mask(Position<N> front, Position<N> back, bool flag = true) : Mask({front, back}, flag) {}

  /**
   * @brief Create a mask from a radius and center position.
   */
  static Mask<N> fromCenter(Index radius = 1, const Position<N>& center = Position<N>::zero(), bool flag = true) {
    return Mask<N>(center - radius, center + radius, flag);
  }

  /**
   * @brief Create a mask from a ball with (pseudo-)norm L0, L1 or L2.
   * @tparam P The norm power
   */
  template <Index P>
  static Mask<N> ball(double radius = 1, const Position<N>& center = Position<N>::zero()) {
    auto out = Mask<N>::fromCenter(radius, center, false);
    const auto radiusPow = std::pow(radius, P);
    auto it = out.m_flags.begin();
    for (const auto& p : out.box() - center) {
      if (norm<P>(p) <= radiusPow) {
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
  const Position<N>& shape() const {
    return m_flags.shape();
  }

  /**
   * @brief Get the number of dimensions.
   */
  Index dimension() const {
    return m_flags.dimension();
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

  /// @group_elements

  /**
   * @brief Get an iterator to the beginning.
   */
  Iterator begin() const {
    return Iterator::begin(*this);
  }

  /**
   * @brief Get an iterator to the end.
   */
  Iterator end() const {
    return Iterator::end(*this);
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
   * @brief Check whether a position is set in the mask.
   */
  bool operator[](const Position<N>& position) const {
    return m_box[position] && m_flags[position - m_box.front()];
  }

  /**
   * @brief Set or unset a position in the mask.
   */
  bool& operator[](const Position<N>& position) {
    // FIXME check bounds
    return m_flags[position - m_box.front()];
  }

  /// @group_modifiers

  /**
   * @brief Translate the mask by a given vector.
   */
  Mask<N>& operator+=(const Position<N>& vector) {
    m_box += vector;
    return *this;
  }

  /**
   * @brief Translate the mask by the opposite of a given vector.
   */
  Mask<N>& operator-=(const Position<N>& vector) {
    m_box -= vector;
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
  Raster<bool, N> m_flags;
};

/**
 * @relates Mask
 * @brief Get the bounding box of a mask.
 */
template <Index N>
inline const Box<N>& box(const Mask<N>& region) {
  return region.box();
}

/**
 * @relates Mask
 * @brief Clamp a mask inside a bounding box.
 */
template <Index N>
inline Mask<N> operator&(const Mask<N>& region, const Box<N>& bounds) {
  const auto box = region.box() & bounds;
  Mask<N> out(box, false);
  for (const auto& p : box) { // FIXME optimize
    out[p] = region[p];
  }
  return out;
}

} // namespace Linx

#include "LinxCore/impl/MaskIterator.h"

#endif
