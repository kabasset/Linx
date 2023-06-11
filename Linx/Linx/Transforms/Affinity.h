// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_AFFINITY_H
#define _LINXTRANSFORMS_AFFINITY_H

#include "Linx/Data/Vector.h"

#include <eigen3/Eigen/Core>

namespace Linx {

/**
 * @brief Geometrical affinity transform (translation, scaling, rotation).
 */
template <Index N = 2>
class Affinity {
private:
  using EigenMatrix = Eigen::Matrix<double, N, N, Eigen::RowMajor>;
  using EigenVector = Eigen::Matrix<double, N, 1>;
  using EigenDiagonal = Eigen::DiagonalMatrix<double, N>;

public:
  /**
   * @brief Create an affinity around given center.
   */
  explicit Affinity(const Vector<double, N>& center = Vector<double, N>::zero()) :
      m_map(EigenMatrix::Identity(center.size(), center.size())), m_translation(EigenVector::Zero(center.size())),
      m_center(toEigenVector(center)) {}

  /**
   * @brief Create a translation.
   */
  static Affinity translation(const Vector<double, N>& vector) {
    Affinity out;
    out += vector;
    return out;
  }

  /**
   * @brief Create a scaling.
   */
  static Affinity
  scaling(const Vector<double, N>& vector, const Vector<double, N>& center = Vector<double, N>::zero()) {
    Affinity out(center);
    out *= vector;
    return out;
  }

  /**
   * @brief Create an isotropic scaling.
   */
  static Affinity scaling(double scalar, const Vector<double, N>& center = Vector<double, N>::zero()) {
    Affinity out(center);
    out *= scalar;
    return out;
  }

  /**
   * @brief Create a rotation by an angle given in radians.
   */
  static Affinity rotationRadians(
      double angle,
      Index from = 0,
      Index to = 1,
      const Vector<double, N>& center = Vector<double, N>::zero()) {
    Affinity out(center);
    out.rotateRadians(angle, from, to);
    return out;
  }

  /**
   * @brief Create a rotation by an angle given in degrees.
   */
  static Affinity rotationDegrees(
      double angle,
      Index from = 0,
      Index to = 1,
      const Vector<double, N>& center = Vector<double, N>::zero()) {
    Affinity out(center);
    out.rotateDegrees(angle, from, to);
    return out;
  }

  /**
   * @brief Translate by a given value along all axes.
   */
  Affinity& operator+=(double scalar) {
    m_translation += scalar;
    return *this;
  }

  /**
   * @brief Translate by a given vector.
   */
  Affinity& operator+=(const Vector<double, N>& vector) {
    if (not vector.isZero()) {
      m_translation += toEigenVector(vector);
    }
    return *this;
  }

  /**
   * @brief Translate by the opposite of a given value along all axes.
   */
  Affinity& operator-=(double scalar) {
    m_translation -= scalar;
    return *this;
  }

  /**
   * @brief Translate by a the opposite of a given vector.
   */
  Affinity& operator-=(const Vector<double, N>& vector) {
    if (not vector.isZero()) {
      m_translation -= toEigenVector(vector);
    }
    return *this;
  }

  /**
   * @brief Scale isotropically by a given factor.
   */
  Affinity& operator*=(double value) {
    m_translation -= m_center;
    m_map *= EigenVector::Fill(value).asDiagonal(value);
    m_translation += m_center;
    return *this;
  }

  /**
   * @brief Scale by a given vector of factors.
   */
  Affinity& operator*=(const Vector<double, N>& vector) {
    if (not vector.isOne()) {
      m_translation -= m_center;
      m_map *= toEigenVector(vector).asDiagonal();
      m_translation += m_center;
    }
    return *this;
  }

  /**
   * @brief Scale by a the inverse of given factor along all axes.
   */
  Affinity& operator/=(double value) {
    return operator*=(1. / value);
  }

  /**
   * @brief Scale by the inverse of a given vector of factors.
   */
  Affinity& operator/=(const Vector<double, N>& vector) {
    if (not vector.isOne()) {
      m_translation -= m_center;
      m_map *= toEigenVector(vector).cwiseInverse().asDiagonal();
      m_translation += m_center;
    }
    return *this;
  }

  /**
   * @brief Rotate by an angle given in radians from a given axis to a given axis.
   */
  Affinity& rotateRadians(double angle, Index from = 0, Index to = 1) {
    if (angle != 0) {
      m_translation -= m_center;
      EigenMatrix rotation = EigenMatrix::Identity();
      const auto sin = std::sin(angle);
      const auto cos = std::cos(angle);
      rotation(from, from) = cos;
      rotation(from, to) = -sin;
      rotation(to, from) = sin;
      rotation(to, to) = cos;
      m_map *= rotation;
      m_translation += m_center;
    }
    return *this;
  }

  /**
   * @brief Rotate by an angle given in degrees from a given axis to a given axis.
   */
  Affinity& rotateDegrees(double angle, Index from = 0, Index to = 1) {
    return rotateRadians(Linx::pi<double>() / 180. * angle, from, to);
  }

  /**
   * @brief Inverse the transform.
   */
  Affinity& inverse() {
    m_map = m_map.inverse();
    m_translation = -m_map * m_translation;
    return *this;
  }

  /**
   * @brief Apply the transform to an input vector.
   */
  template <typename T>
  Vector<double, N> operator[](const Vector<T, N>& in) const {
    return Vector<double, N>(m_translation + m_map * toEigenVector(in));
    // TODO faster without cast?
    // Vector<double, N> out(m_translation);
    // auto it = out.begin();
    // for (auto row : m_map.rowwise()) {
    //   *it += std::inner_product(row.begin(), row.end(), in.begin(), T()); // FIXME init = m_translation(i)?
    //   ++it;
    // }
  }

  /**
   * @brief Apply the transform to an input interpolator.
   * 
   * The domain of the output parameter is used to decide which positions to take into account.
   * If positions outside the input domain are required, then `in` must be an extrapolator, too.
   */
  template <typename TInterpolator, typename TRaster>
  TRaster& transform(const TInterpolator& in, TRaster& out) const {
    auto it = out.begin();
    for (const auto& p : out.domain()) {
      *it = in[operator()(p)];
      ++it;
    }
    return out;
  }

private:
  static EigenVector toEigenVector(const Vector<double, N>& in) {
    EigenVector out(in.size());
    std::copy(in.begin(), in.end(), out.begin());
    return out;
  }

  EigenMatrix m_map;
  EigenVector m_translation;
  EigenVector m_center;
};

} // namespace Linx

#endif
