// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_BOX_ARITHMETICS_H
#define LINX_DATA_BOX_ARITHMETICS_H

#include "Linx/Base/Slice.h"
#include "Linx/Base/Types.h" // Specialization
#include "Linx/Data/concepts/Region.h"

#include <concepts>

namespace Linx {

/**
 * @brief Box shifted by a given vector or scalar.
 */
constexpr auto operator+(const Specialization<Box> auto& lhs, const auto& rhs)
{
  return Box(lhs.start() + rhs, lhs.stop() + rhs);
}

/**
 * @brief Box shifted by the opposite of a given vector or scalar.
 */
constexpr auto operator-(const Specialization<Box> auto& lhs, const auto& rhs)
{
  return Box(lhs.start() - rhs, lhs.stop() - rhs);
}

/**
 * @brief Dilation of a box by a given regular margin.
 */
template <Specialization<Box> TBox>
constexpr auto dilate(const TBox& box, const std::convertible_to<typename TBox::size_type> auto& margin)
{
  return Box(box.start() - margin, box.stop() + margin);
}

/**
 * @brief Erosion of a box by a given regular margin.
 */
template <Specialization<Box> TBox>
constexpr auto erode(const TBox& box, const std::convertible_to<typename TBox::size_type> auto& margin)
{
  return Box(box.start() + margin, box.stop() - margin);
}

/**
 * @brief Dilation of a box by a given margin.
 * 
 * The resulting region starts at `box.start() + bbox(margin).start()`
 * and stops at `box.stop() + (box(margin).stop() - 1)`.
 */
constexpr auto dilate(const Specialization<Box> auto& box, const BoundedRegion auto& margin)
{
  auto margin_box = bbox(margin);
  return Box(box.start() + margin_box.start(), box.stop() + (margin_box.stop() - 1));
}

/**
 * @brief Erosion of a box by a given margin.
 * 
 * The resulting region starts at `box.start() - bbox(margin).start()`
 * and stops at `box.stop() - (box(margin).stop() - 1)`.
 */
constexpr auto erode(const Specialization<Box> auto& box, const BoundedRegion auto& margin)
{
  auto margin_box = bbox(margin);
  return Box(box.start() - margin_box.start(), box.stop() - (margin_box.stop() - 1));
}

/**
 * @brief Intersection of two boxes.
 */
constexpr auto operator&(const Specialization<Box> auto& lhs, const Specialization<Box> auto& rhs)
{
  return Box(max(lhs.start(), rhs.start()), min(lhs.stop(), rhs.stop()));
}

/**
 * @relatesalso Slice
 * @relatesalso Box
 * @brief Intersection of a slice and a box or slice.
 * 
 * The region may be of higher rank than the slice: extra dimensions are ignored.
 */
constexpr auto operator&(const Specialization<Slice> auto& slice, const auto& region)
// FIXME requires region.start(i), regions.stop(i)? Box or Slice = concept Slicing?
{
  constexpr auto last = LINX_DECLTYPE(slice)::n - 1;
  if constexpr (last == 0) {
    return clamp(slice, region.start(0), region.stop(0));
  } else {
    return Slice(Forward(), slice.lower() & region, clamp(slice.last(), region.start(last), region.stop(last)));
  }
}

/**
 * @relatesalso Box
 * @brief Bounding box of two boxes.
 */
constexpr auto bbox(const Specialization<Box> auto& lhs, const Specialization<Box> auto& rhs)
{
  return Box(min(lhs.start(), rhs.start()), max(lhs.stop(), rhs.stop()));
}

} // namespace Linx

#endif
