// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_OUTPUTSTREAM_H
#define LINX_BASE_OUTPUTSTREAM_H

namespace Linx {

/**
 * @brief I/O manipulator to select the number of printed edge elements.
 */
class PrintLimit {
public:

  /**
   * @brief Constructor.
   */
  explicit constexpr PrintLimit(long edge = 0) : m_edge(edge) {}

  /**
   * @brief Access the maximum number of edge values.
   */
  static long& edge(std::ios_base& b)
  {
    static int const index = std::ios_base::xalloc();
    return b.iword(index);
  }

  /**
   * @brief Stream insertion.
   */
  friend std::ostream& operator<<(std::ostream& os, const PrintLimit& limit)
  {
    PrintLimit::edge(os) = limit.m_edge;
    return os;
  }

private:

  const long m_edge; ///< The maximum number of edge values
};

} // namespace Linx

#endif
