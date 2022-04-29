// Copyright (C) 2022, CNES
// This file is part of KRaster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _KRASTERUTILS_EXCEPTIONS_H
#define _KRASTERUTILS_EXCEPTIONS_H

#include <exception>
#include <string>
#include <utility> // pair // FIXME replace with Segment

namespace Kast {

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
   * @param message Input message
   */
  explicit Exception(const std::string& message) :
      std::exception(), m_prefix("KRaster error: "), m_message(m_prefix + message) {}

  /**
   * @brief Output message.
   */
  const char* what() const noexcept override {
    return m_message.c_str();
  }

  /**
   * @brief Append a given line to the message.
   * @param line The line to be appended
   * @param indent Some indentation level
   */
  Exception& append(const std::string& line, std::size_t indent = 0) {
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
 * @brief Exception thrown if a value lies out of given bounds.
 */
class OutOfBoundsError : public Exception {

public:
  /**
   * @brief Constructor.
   * @details
   * The error message is of the form "<prefix>: <value> not in (<min>, <max>)".
   */
  OutOfBoundsError(const std::string& prefix, long value, std::pair<long, long> bounds) :
      Exception(
          prefix + ": " + std::to_string(value) + " not in (" + std::to_string(bounds.first) + ", " +
          std::to_string(bounds.second) + ")") {}

  /**
   * @brief Throw if a value lies out of given bounds, included.
   */
  static void mayThrow(const std::string& prefix, long value, std::pair<long, long> bounds) {
    if (value < bounds.first || value > bounds.second) {
      throw OutOfBoundsError(prefix, value, bounds);
    }
  }
};

} // namespace Kast

#endif // _KRASTERUTILS_EXCEPTIONS_H
