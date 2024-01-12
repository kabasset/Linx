/// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINX_IO_H
#define _LINX_IO_H

#include "Linx/Io/Fits.h"
#include "Linx/Io/Temporary.h"

#include <filesystem>
#include <string>

namespace Linx {

/**
 * @brief Read raster from a file.
 */
template <typename T, Index N = 2>
Raster<T, N> read(const std::filesystem::path& path, Index index = 0)
{
  try {
    return Fits(path).read<Raster<T, N>>(index);
  } catch (FileFormatError&) {
    throw FileFormatError("No suitable reader", path);
  }
}

/**
 * @brief Write a raster to a file.
 */
template <typename TRaster>
void write(const TRaster& in, const std::filesystem::path& path, char mode = 'x')
{
  try {
    return Fits(path).write(in, mode);
  } catch (FileFormatError&) {
    throw FileFormatError("No suitable writer ", path);
  }
}

} // namespace Linx

#endif
