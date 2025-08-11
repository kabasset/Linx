// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_BOX_TYPES_H
#define LINX_DATA_BOX_TYPES_H

#include "Linx/Data/Box/creation.h"

namespace Linx {

/**
 * @brief Box which starts at origin.
 */
template <typename TStop>
using Shape = decltype(Box(std::declval<Vector<TStop>>()));

} // namespace Linx

#endif
