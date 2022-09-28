// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLCORE_IMPL_PATCHITERATOR_H
#define _LITLCORE_IMPL_PATCHITERATOR_H

#include "LitlCore/Patch.h"
#include "LitlCore/impl/PatchIndexing.h"

namespace Litl {

template <typename TParent, typename TRegion>
template <typename T>
class PositionBasedIndexing<TParent, TRegion>::Iterator : public std::iterator<std::forward_iterator_tag, T> {

public:
  using Value = T;

  using PositionIterator = decltype(std::declval<const TRegion>().begin());

  Iterator(TParent& parent, PositionIterator it) : m_parent(parent), m_current(std::move(it)) {}

  Value& operator*() const {
    return m_parent[*m_current];
  }

  Value* operator->() const {
    return &m_parent[*m_current];
  }

  Iterator& operator++() {
    ++m_current;
    return *this;
  }

  Iterator operator++(int) {
    auto out = *this;
    ++(*this);
    return out;
  }

  bool operator==(const Iterator& rhs) const {
    return m_current == rhs.m_current;
  }

  bool operator!=(const Iterator& rhs) const {
    return m_current != rhs.m_current;
  }

private:
  TParent& m_parent;
  PositionIterator m_current;
};

template <typename TParent, typename TRegion>
template <typename T>
class StrideBasedIndexing<TParent, TRegion>::Iterator : public std::iterator<std::forward_iterator_tag, T> {

public:
  using Value = T;
  static constexpr Index Dimension = TParent::Dimension;

  /**
   * @brief Constructor.
   * @param step The stride along axis 0
   * @param width The length along axis 0
   * @param offset An iterator to the beginning of line offsets
   */
  Iterator(Value* front, Index step, Index width, const Index* offset) :
      m_step(step), m_width(width), m_front(front), m_eol(m_front + m_width), m_offsetIt(offset),
      m_current(m_front + *m_offsetIt) {}

  /**
   * @brief Dereference operator.
   */
  Value& operator*() const {
    return *m_current;
  }

  /**
   * @brief Arrow operator.
   */
  Value* operator->() const {
    return m_current;
  }

  /**
   * @brief Increment operator.
   */
  Iterator& operator++() {
    m_current += m_step;
    if (m_current < m_eol) {
      return *this;
    }
    m_current = m_front + *(++m_offsetIt);
    // FIXME cannot dereference if m_offsetIt == m_offsetEnd => size + 1 to avoid branching?
    m_eol = m_current + m_width;
    return *this;
  }

  /**
   * @brief Increment operator.
   */
  Iterator operator++(int) {
    auto out = *this;
    ++(*this);
    return out;
  }

  /**
   * @brief Equality operator.
   */
  bool operator==(const Iterator& rhs) const {
    return m_offsetIt == rhs.m_offsetIt;
  }

  /**
   * @brief Non equality operator.
   */
  bool operator!=(const Iterator& rhs) const {
    return m_offsetIt != rhs.m_offsetIt;
  }

private:
  /**
   * @brief The stride along a row.
   */
  Index m_step;

  /**
   * @brief The distance between the row beginning and end.
   */
  Index m_width;

  /**
   * @brief The front pointer.
   */
  Value* m_front;

  /**
   * @brief The current end of line pointer.
   */
  Value* m_eol;

  /**
   * @brief A beginning of line offset iterator.
   */
  const Index* m_offsetIt;

  /**
   * @brief The current pointer.
   */
  Value* m_current;
};

template <typename TParent, typename TRegion>
template <typename T>
class OffsetBasedIndexing<TParent, TRegion>::Iterator : public std::iterator<std::forward_iterator_tag, T> {

public:
  using Value = T;

  Iterator(Value* data, const Index* current) : m_data(data), m_current(current) {}

  Value& operator*() const {
    return *(m_data + *m_current);
  }

  Value* operator->() const {
    return m_data + *m_current;
  }

  Iterator& operator++() {
    ++m_current;
    return *this;
  }

  Iterator operator++(int) {
    auto out = *this;
    ++(*this);
    return out;
  }

  bool operator==(const Iterator& rhs) const {
    return m_current == rhs.m_current;
  }

  bool operator!=(const Iterator& rhs) const {
    return m_current != rhs.m_current;
  }

private:
  Value* m_data;
  const Index* m_current;
};

} // namespace Litl

#endif
