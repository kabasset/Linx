// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_IO_EXCEPTIONS_H
#define LINX_IO_EXCEPTIONS_H

#include "Linx/Base/Exceptions.h"

#include <filesystem>

namespace Linx {

/**
 * @brief File access modes.
 */
enum class FileMode : char {
  Read = 'r', ///< Read an existing file, may throw `FileNotFound`
  Edit = 'e', ///< Edit an existing file, may throw `FileNotFound`
  Create = 'x', ///< Create a new file, may throw `PathAlreadyExists`
  Overwrite = 'w', ///< Overwrite an existing file, or create a new file
  Write = 'a', ///< Edit an existing file, or create a new file
  Temporary = 't' ///< Create a new file, destroy it after use, may throw `PathAlreadyExists`
};

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

  /**
   * @brief Throw if access mode expects a file, which is not found.
   */
  static void may_throw(const std::filesystem::path& path, FileMode mode)
  {
    switch (mode) {
      case FileMode::Read:
      case FileMode::Edit:
        may_throw(path);
      default:
        return;
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

  /**
   * @brief Throw if access mode expects path does not exist, while it does.
   */
  static void may_throw(const std::filesystem::path& path, FileMode mode)
  {
    switch (mode) {
      case FileMode::Create:
      case FileMode::Temporary:
        may_throw(path);
      default:
        return;
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
