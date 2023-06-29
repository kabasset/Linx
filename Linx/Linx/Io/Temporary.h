/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXIO_TEMPORARY_H
#define _LINXIO_TEMPORARY_H

#include <filesystem>
#include <string>

namespace Linx {

/**
 * @brief A path which is removed at destruction.
 */
class TemporaryPath {
public:
  /**
   * @brief Constructor.
   * 
   * The path is in a suitable temporary directory.
   */
  explicit TemporaryPath(std::string name) : m_path(std::filesystem::temp_directory_path() / name) {}

  /**
   * @brief Destructor.
   * 
   * Removes the path.
   */
  virtual ~TemporaryPath() {
    std::filesystem::remove_all(m_path);
  }

  /**
   * @brief Get the path.
   */
  operator std::filesystem::path() const {
    return m_path;
  }

  std::string string() const {
    return m_path.string();
  }

private:
  std::filesystem::path m_path;
};

} // namespace Linx

#endif
