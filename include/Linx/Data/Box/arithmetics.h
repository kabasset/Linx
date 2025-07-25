// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_BOX_ARITHMETICS_H
#define LINX_DATA_BOX_ARITHMETICS_H

namespace Linx {

/**
 * @brief Create the dilation of a box by a given margin.
 */
template <typename TStart, typename TStop>
constexpr auto
dilate(const Box<TStart, TStop>& box, const std::convertible_to<typename Box<TStart, TStop>::size_type> auto& margin)
{
  return Box(box.start() - margin, box.stop() + margin);
}

/**
 * @brief Create the erosion of a box by a given margin.
 */
template <typename TStart, typename TStop>
constexpr auto
erode(const Box<TStart, TStop>& box, const std::convertible_to<typename Box<TStart, TStop>::size_type> auto& margin)
{
  return Box(box.start() + margin, box.stop() - margin);
}

/**
 * @brief Create the dilation of a box by a given margin.
 */
template <typename TStart, typename TStop, typename TRhs> // TODO BoundedRegion TRhs
  requires requires(const TRhs& rhs)
  {
    bbox(rhs); // TODO -> Box
  }
constexpr auto dilate(const Box<TStart, TStop>& box, const TRhs& margin)
{
  auto margin_box = bbox(margin);
  return Box(box.start() + margin_box.start(), box.stop() + (margin_box.stop() - 1));
}

/**
 * @brief Create the erosion of a box by a given margin.
 */
template <typename TStart, typename TStop, typename TRhs> // TODO BoundedRegion TRhs
  requires requires(const TRhs& rhs)
  {
    bbox(rhs); // TODO -> Box
  }
constexpr auto erode(const Box<TStart, TStop>& box, const TRhs& margin)
{
  auto margin_box = bbox(margin);
  return Box(box.start() - margin_box.start(), box.stop() - (margin_box.stop() - 1));
}

/**
 * @relatesalso Slice
 * @relatesalso Box
 * @brief Make a slice clamped by a region.
 * 
 * The region may be of higher rank than the slice: extra dimensions are ignored.
 */
template <typename T, typename... TFuncs>
constexpr auto
operator&(const Slice<T, TFuncs...>& slice, const auto& region) // FIXME requires region.start(i), regions.stop(i)
{
  constexpr auto last = sizeof...(TFuncs) - 1;
  if constexpr (last == 0) {
    return clamp(slice, region.start(0), region.stop(0));
  } else {
    return Slice(Forward(), slice.lower() & region, clamp(slice.last(), region.start(last), region.stop(last)));
  }
}

} // namespace Linx

#endif
