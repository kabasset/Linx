// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_BOX_BOXUNION_H
#define LINX_DATA_BOX_BOXUNION_H

namespace Linx {

/**
 * @brief A region made of a collection of disjoint boxes.
 *
 * The region can be iterated with `for_each()`.
 */
template <typename TBox>
class BoxUnion {
public:

  using value_type = typename TBox::value_type; ///< The position type
  using index_type = typename TBox::index_type; ///< The coefficient type
  static constexpr auto n = TBox::n; ///< The rank parameter

  /**
   * @brief Constructor.
   */
  BoxUnion() : m_boxes {} {}

  /**
   * @brief Constructor.
   */
  BoxUnion(auto&& box) : m_boxes {LINX_MOVE(box)} {}

  /**
   * @brief The vector of the individual boxes making up the region.
   */
  const auto boxes() const
  {
    return m_boxes;
  }

  /**
   * @brief The number of elements in the region.
   * 
   * @warning This is not the number of individual boxes!
   */
  auto size() const
  {
    index_type out = 0;
    for (const auto& box : m_boxes) {
      out += box.size();
    }
    return out;
  }

  /**
   * @brief Add a new box to the region.
   * 
   * If the box is empty, it is not added.
   * No other tests are performed by this function;
   * The user is responsible for ensuring that the box it is disjoint from all existing boxes.
   */
  bool emplace_disjoint(auto&&... args)
  {
    auto box = TBox(LINX_FORWARD(args)...);
    if (box.size() <= 0) { // FIXME box.empty()
      return false;
    }
    m_boxes.emplace_back(LINX_MOVE(box));
    return true;
  }

private:

  std::vector<TBox> m_boxes; ///< The individual boxes
};

/**
 * @brief Iterate over a union of boxes.
 * 
 * The function is applied to each element of each box in the region.
 */
template <typename TSpace, typename TBox>
void for_each(const std::string& label, const BoxUnion<TBox>& region, auto&& func)
{
  for (const auto& box : region.boxes()) {
    for_each<TSpace>(label, box, LINX_FORWARD(func));
  }
}

/**
 * @relatesalso GBox
 * @brief Compute the set difference between two boxes.
 * 
 * The left-hand side box is assumed to be non-empty.
 * The output is a union of boxes representing
 * the elements contained in the left-hand side box which are not contained in the right-hand side box.
 */
template <typename TBox>
BoxUnion<TBox> set_difference(const TBox& lhs, const auto& rhs)
{
  // Inner box
  const auto inner = lhs & rhs;
  if (inner.size() == 0) {
    return BoxUnion<TBox>(lhs);
  }

  // Processed region
  auto current_start = inner.start();
  auto current_stop = inner.stop();

  auto out = BoxUnion<TBox>();
  for (std::size_t i = 0; i < lhs.rank(); ++i) { // FIXME rank_type?
    // Add box, grow current region frontwards
    const auto start_i = lhs.start(i) - current_start[i];
    if (start_i < 0) {
      auto start = current_start;
      auto stop = current_stop;
      stop[i] = current_start[i];
      start[i] = current_start[i] += start_i;
      out.emplace_disjoint(LINX_MOVE(start), LINX_MOVE(stop));
    }

    // Add box, grow current region backwards
    const auto stop_i = lhs.stop(i) - current_stop[i];
    if (stop_i > 0) {
      auto start = current_start;
      auto stop = current_stop;
      start[i] = current_stop[i];
      stop[i] = current_stop[i] += stop_i;
      out.emplace_disjoint(LINX_MOVE(start), LINX_MOVE(stop));
    }
  }

  return out;
}

/**
 * @brief Compute the union of two boxes.
 * 
 * The left-hand side box is assumed to be non-empty.
 * The output is a union of boxes representing
 * the elements contained in the left-hand side box and/or in the right-hand side box.
 * The left-hand side box is one of the boxes in the output.
 */
template <typename TBox>
BoxUnion<TBox> operator|(const TBox& lhs, const auto& rhs)
{
  auto out = set_difference(rhs, lhs); // Split `rhs`
  out.emplace_disjoint(lhs); // Keep `lhs` as is
  return out;
}

} // namespace Linx

#endif
