// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_AFFINITY_H
#define _LITLTRANSFORMS_AFFINITY_H

#include "LitlRaster/Matrix.h"
#include "LitlRaster/Vector.h"

namespace Litl {

template <Index N = 2>
class Affinity {

public:
  explicit Affinity(const Vector<double, N>& center = Vector<double, N>::zero()) :
      m_map(Matrix<double, N>::identity()), m_translation(), m_center(center) {}

  Affinity& translate(const Vector<double, N>& offset) {
    if (not offset.isZero()) {
      m_translation += offset;
    }
    return *this;
  }

  Affinity& scale(const Vector<double, N>& factor) {
    if (not factor.isOne()) {
      translate(-m_center);
      m_map *= Matrix<double, N>::diagonal(factor);
      translate(m_center);
    }
    return *this;
  }

  Affinity& rotate(double radians, Index axis = 2) {
    if (radians != 0) {
      translate(-m_center);
      auto rotation = Matrix<double, N>::identity();
      // FIXME sin, cos
      m_map *= rotation;
      translate(m_center);
    }
    return *this;
  }

  Affinity& inverse() {
    m_map = m_map.inverse();
    m_translation = -m_map * m_translation;
    return *this;
  }

  Affinity& operator*=(const Matrix<double, N>& map) {
    m_map *= map;
    return *this;
  }

  Affinity& operator+=(const Vector<double, N>& translation) {
    m_translation += translation;
    return *this;
  }

  template <typename T, Index M = N>
  Vector<double, M> operator()(const Vector<T, M>& in) const {
    return m_translation + m_map * in;
  }

  template <typename TInterpolator, typename TRaster>
  TRaster& transformTo(const TInterpolator& in, TRaster& out) const {
    for (const auto& p : out.domain()) {
      out[p] = in[operator()(p)];
    }
    return out;
  }

private:
  Matrix<double, N> m_map;
  Vector<double, N> m_translation;
  Vector<double, N> m_center;
};

} // namespace Litl

#endif
