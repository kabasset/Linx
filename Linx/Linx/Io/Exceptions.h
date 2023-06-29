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
  /**
   * @brief Constructor.
   */
  FileNotFoundError(const std::filesystem::path& path) : Exception("File does not exist") {
    append(path);
  }

  /**
   * @brief Throw if a given path is not a file.
   */
  static void mayThrow(const std::filesystem::path& path) {
    if (not std::filesystem::is_regular_file(path)) {
      throw FileNotFoundError(path);
    }
  }
};

/**
 * @brief Exception thrown when a path already exists.
 */
class PathExistsError : public Exception {
public:
  /**
   * @brief Constructor.
   */
  PathExistsError(const std::filesystem::path& path) : Exception("Path already exists") {
    append(path);
  }

  /**
   * @brief Throw if a given path is not a file.
   */
  static void mayThrow(const std::filesystem::path& path) {
    if (std::filesystem::exists(path)) {
      throw PathExistsError(path);
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
