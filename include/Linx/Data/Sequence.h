// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_SEQUENCE_H
#define LINX_DATA_SEQUENCE_H

#include "Linx/Base/Containers.h"
#include "Linx/Data/Image.h"

namespace Linx {

template <typename T, int N, typename TView = SequenceContainer<T, N>>
using Sequence = Image<T, Shape<std::integer_sequence<int, N>>, TView>; // FIXME enable operator[] and range ops

} // namespace Linx

#include "Linx/Data/Sequence/creation.h"
#include "Linx/Data/Sequence/funcs.h"

#endif
