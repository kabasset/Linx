// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_BOX_TYPES_H
#define LINX_DATA_BOX_TYPES_H

namespace Linx {

/**
 * @brief Box which starts at origin.
 */
template <typename TStop>
using Shape = Box<std::integer_sequence<int>, TStop>;

} // namespace Linx

#endif
