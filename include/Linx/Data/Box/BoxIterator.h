// SPDX-FileCopyrightText: Copyright (C) 2022-2025, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_BOX_BOXITERATOR_H
#define LINX_DATA_BOX_BOXITERATOR_H

namespace Linx {

/**
 * @brief Layout left box iterator.
 */
template <typename TStart, typename TStop>
class BoxIterator {
public:

  using iterator_tag = std::input_iterator_tag; ///< The tye of iterator
  using element_type = typename Box<TStart, TStop>::element_type; ///< The element type
  using value_type = const element_type; ///< The value type
  using size_type = typename element_type::element_type; ///< The coefficient type
  using reference = value_type&; ///< The reference type
  using pointer = value_type*; ///< The pointer type

  /**
   * @brief Constructor.
   */
  explicit constexpr BoxIterator(const Box<TStart, TStop>& region, element_type current) :
      m_region(region),
      m_current(LINX_MOVE(current))
  {}

  /**
   * @brief Dereference operator.
   */
  constexpr reference operator*() const
  {
    return m_current;
  }

  /**
   * @brief Arrow operator.
   */
  constexpr pointer operator->() const
  {
    return &m_current;
  }

  /**
   * @brief Increment operator.
   */
  constexpr BoxIterator& operator++()
  {
    ++m_current[0];
    for (std::size_t i = 0; i < m_current.size() - 1; ++i) {
      if (m_current[i] >= m_region.stop(i)) {
        m_current[i] = m_region.start(i);
        ++m_current[i + 1];
      }
    }
    return *this;
  }

  /**
   * @brief Increment operator.
   */
  constexpr BoxIterator operator++(int)
  {
    auto out = *this;
    ++*this;
    return out;
  }

  /**
   * @brief Equality operator.
   */
  constexpr bool operator==(const BoxIterator& rhs) const
  {
    return m_current == rhs.m_current;
  }

  /**
   * @brief Inequality operator.
   */
  constexpr bool operator!=(const BoxIterator& rhs) const
  {
    return m_current != rhs.m_current;
  }

  /**
   * @brief Sentinel equality operator.
   */
  constexpr bool operator==(size_type rhs) const
  {
    return m_current[m_current.size() - 1] == rhs;
  }

  /**
   * @brief Sentinel inequality operator.
   */
  constexpr bool operator!=(size_type rhs) const
  {
    return not(*this == rhs);
  }

private:

  const Box<TStart, TStop>& m_region; ///< The box
  element_type m_current; ///< The current position
};

/**
 * @brief Box iterator to the beginning.
 */
template <typename TStart, typename TStop>
constexpr auto begin(const Box<TStart, TStop>& box)
{
  typename Box<TStart, TStop>::element_type current = box.start();
  return BoxIterator<TStart, TStop>(box, LINX_MOVE(current));
}

/**
 * @brief Box sentinel.
 */
template <typename TStart, typename TStop>
constexpr auto end(const Box<TStart, TStop>& box)
{
  return box.ssize() > 0 ? box.stop(box.rank() - 1) : box.start(box.rank() - 1);
}

} // namespace Linx

#endif
