// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_CONCEPTS_REGION_H
#define LINX_DATA_CONCEPTS_REGION_H

#include "Linx/Base/concepts/Arithmetic.h"

namespace Linx {

/**
 * @brief Collection of positions, which can be iterated.
 * 
 * If the region can be shifted, it is a window.
 * 
 * @see `Window`
 */
template <typename T>
concept Region = requires(const T region) {
  T::n;
  typename T::size_type;
  typename T::value_type;
  std::size(region);
  // region & Box<typename T::size_type, T::n>(); // FIXME recursive
  for_each("", region, [](auto... is) {});
};

/**
 * @brief Object with finite bounding box.
 */
template <typename T>
concept BoundedRegion = requires(const T region) {
  bbox(region); // TODO return Box
};

/**
 * @brief Additive region.
 * 
 * Patches whose domain are windows can be translated.
 * 
 * @see `Region`
 */
template <typename T>
concept Window = Region<T> && Additive<T, typename T::value_type> && Additive<T, typename T::size_type>;

} // namespace Linx

#endif
