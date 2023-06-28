/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXIO_EXCEPTION_H
#define _LINXIO_EXCEPTION_H

#include "Linx/Base/Exceptions.h"

#include <filesystem>
#include <string>

namespace Linx {

/**
 * @brief Exception thrown when a file does not exist.
 */
class FileNotFoundError : public Exception {
public:
  FileNotFoundError(const std::filesystem::path& path) : Exception("File does not exist") {
    append(path);
  }

  static void mayThrow(const std::filesystem::path& path) {
    if (not std::filesystem::is_regular_file(path)) {
      throw FileNotFoundError(path);
    }
  }
};

/**
 * @brief Exception thrown when a file cannot be handled due to format issues.
 */
class FileFormatError : public Exception {
public:
  FileFormatError(const std::string& message, const std::filesystem::path& path) :
      Exception("File format error", message) {
    append(path);
  }
};

} // namespace Linx

#endif
