// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_IO_EXCEPTIONS_H
#define LINX_IO_EXCEPTIONS_H

#include "Linx/Base/Exceptions.h"

#include <filesystem>

namespace Linx {

class FileNotFound : public Exception {
public:

  /**
   * @brief Constructor.
   */
  FileNotFound(const std::filesystem::path& path) : Exception("File does not exist")
  {
    append(path);
  }

  /**
   * @brief Throw if a given path is not a file.
   */
  static void may_throw(const std::filesystem::path& path)
  {
    if (not std::filesystem::is_regular_file(path)) {
      throw FileNotFound(path);
    }
  }
};

/**
 * @brief Exception thrown when a path already exists.
 */
class PathAlreadyExists : public Exception {
public:

  /**
   * @brief Constructor.
   */
  PathAlreadyExists(const std::filesystem::path& path) : Exception("Path already exists")
  {
    append(path);
  }

  /**
   * @brief Throw if a given path is not a file.
   */
  static void may_throw(const std::filesystem::path& path)
  {
    if (std::filesystem::exists(path)) {
      throw PathAlreadyExists(path);
    }
  }
};

/**
 * @brief Exception thrown when a file cannot be handled due to format issues.
 */
class WrongFileFormat : public Exception {
public:

  WrongFileFormat(const std::string& message, const std::filesystem::path& path) :
      Exception("File format error", message)
  {
    append(path);
  }
};

} // namespace Linx

#endif
