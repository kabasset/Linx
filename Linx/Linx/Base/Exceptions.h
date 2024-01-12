// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXBASE_EXCEPTIONS_H
#define _LINXBASE_EXCEPTIONS_H

#include <exception>
#include <string>
#include <utility> // pair // FIXME replace with Slice?

namespace Linx {

/**
 * @ingroup exceptions
 * @brief Base of all exceptions thrown directly by the library.
 */
class Exception : public std::exception {
public:

  /**
   * @brief Destructor.
   */
  virtual ~Exception() = default;

  /**
   * @brief Constructor.
   * @param message Error message
   */
  explicit Exception(const std::string& message) : Exception("Linx error", message) {}

  /**
   * @brief Constructor.
   * @param prefix Error prefix
   * @param message Error message
   */
  explicit Exception(const std::string& prefix, const std::string& message) :
      std::exception(), m_prefix(prefix), m_message(m_prefix + ": " + message)
  {}

  /**
   * @brief Output message.
   */
  const char* what() const noexcept override
  {
    return m_message.c_str();
  }

  /**
   * @brief Append a given line to the message.
   * @param line The line to be appended
   * @param indent Some indentation level
   */
  Exception& append(const std::string& line, std::size_t indent = 0)
  {
    m_message += "\n";
    for (std::size_t i = 0; i < indent; ++i) {
      m_message += "  ";
    }
    m_message += line;
    return *this;
  }

private:

  const std::string m_prefix;
  std::string m_message;
};

/**
 * @ingroup exceptions
 * @brief Exception thrown when trying to read a null pointer.
 */
class NullPtrError : public Exception {
public:

  /**
   * @brief Constructor.
   */
  NullPtrError(const std::string& message) : Exception("Null pointer error", message) {}

  /**
   * @brief Throw if a given pointer is null.
   */
  void may_throw(const void* ptr, const std::string& message)
  {
    if (not ptr) {
      throw NullPtrError(message);
    }
  }
};

/**
 * @ingroup exceptions
 * @ingroup resampling
 * @brief Exception thrown if a value lies out of given bounds.
 */
class OutOfBoundsError : public Exception {
public:

  /**
   * @brief Constructor.
   * 
   * The error message is of the form "name: <value> not in [<min>, <max>]".
   */
  template <typename T>
  OutOfBoundsError(const std::string& name, T value, std::pair<T, T> bounds) :
      Exception(
          "Out of bounds error",
          name + std::to_string(value) + " not in [" + std::to_string(bounds.first) + ", " +
              std::to_string(bounds.second) + "]")
  {}

  /**
   * @brief Throw if a value lies out of given bounds, included.
   */
  template <typename T>
  static void may_throw(const std::string& name, T value, std::pair<T, T> bounds)
  {
    if (value < bounds.first || value > bounds.second) {
      throw OutOfBoundsError(name, value, bounds);
    }
  }
};

} // namespace Linx

#endif
