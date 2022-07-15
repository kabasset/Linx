// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTYPES_SEGMENT_H
#define _LITLTYPES_SEGMENT_H

#include "LitlTypes/TypeUtils.h"

namespace Litl {

class Segment { // FIXME

public:
  Segment(Index front, Index back) : m_front(front), m_back(back) {}

  static Segment fromSize(Index front, std::size_t size) {
    return Segment(front, size - front - 1);
  }

  Index front() const {
    return m_front;
  }

  Index back() const {
    return m_back;
  }

  std::size_t size() const {
    return m_back - m_front + 1;
  }

private:
  Index m_front;
  Index m_back;
};

} // namespace Litl

#endif
