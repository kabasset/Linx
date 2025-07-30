// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_VECTOR_FUNCS_H
#define LINX_DATA_VECTOR_FUNCS_H

namespace Linx {

/**
 * @brief Stream insertion.
 */
template <typename T>
std::ostream& operator<<(std::ostream& os, const Vector<T>& p)
{
  if (p.size() == 0) {
    return os << "O";
  }

  os << "[" << p(0);
  for (int i = 1; i < p.size(); ++i) {
    os << ", " << p(i);
  }
  return os << "]";
}

} // namespace Linx

#endif
