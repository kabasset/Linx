// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_AFFINITY_H
#define _LINXTRANSFORMS_AFFINITY_H

#include "Linx/Data/Raster.h"
#include "Linx/Data/Vector.h"
#include "Linx/Transforms/Interpolation.h"

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/LU> // inverse

namespace Linx {

/// @cond

// Forward declare for inverse
template <Index N = 2>
class Affinity;

// Forward declare for Affinity::transform()
template <Index N>
Affinity<N> inverse(const Affinity<N>& in);

/// @endcond

/**
 * @ingroup affinity
 * @brief Geometrical affine transform (translation, scaling, rotation).
 * 
 * Affinities transform an input vector `x` into an output vector `y` by applying a linear map `a` (square matrix) and a translation vector `b` as:
 * \code
 * y = a * x + b
 * \endcode
 * 
 * The map typically represents scaling and/or rotation transforms.
 * In geometry, it is common for such transforms to be defined relative to some center `c`,
 * which is not explicit in the matrix representation, but can be integrated as:
 * \code
 * y = a * (x - c) + b + c
 * \endcode
 * 
 * This class makes the center explicit in the constructor.
 * The affinity is then built up by composition.
 * Here is an example to create a 30 degrees-rotation centered in (100, 50) and oriented from axis 1 to axis 0:
 * 
 * \code
 * Affinity<2> affinity({100, 50});
 * affinity.rotate_deg(30, 1, 0);
 * auto y = affinity(x);
 * \code
 * 
 * The affinity can be applied to a raster, or patch, too, e.g. with `operator*()`.
 * For example, let us scale an input raster by a factor 3 around its center:
 * 
 * \code
 * Vector<double> center = in.domain();
 * center /= 2;
 * Affinity<2> affinity(center);
 * affinity *= 3;
 * out = affinity * in;
 * \endcode
 * 
 * This use case is very classical and a shortcut is therefore provided:
 * 
 * \code
 * auto out = scale(in, 3);
 * \endcode
 * 
 * Note that `in` must at least be an interpolator.
 * If values outside its domain are required, then it must also be an extrapolator.
 * 
 * Let us also stress out that the output is of the same shape as the input.
 * It is possible to resample the output with:
 * 
 * \code
 * Raster<int> out(in.shape() * 3);
 * Affinity<2> upscale;
 * upscale *= 3;
 * upscale.transform(in, out);
 * \endcode
 * 
 * Which is equivalent to:
 * 
 * \code
 * out = upsample(in, 3);
 * \endcode
 * 
 * \note
 * This class depends on Eigen
 */
template <Index N>
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
      m_center(to_eigen_vector(center))
  {}

  /**
   * @brief Create a translation.
   */
  static Affinity translation(const Vector<double, N>& vector)
  {
    Affinity out;
    out += vector;
    return out;
  }

  /**
   * @brief Create a scaling.
   */
  static Affinity scaling(const Vector<double, N>& vector, const Vector<double, N>& center = Vector<double, N>::zero())
  {
    Affinity out(center);
    out *= vector;
    return out;
  }

  /**
   * @brief Create an isotropic scaling.
   */
  static Affinity scaling(double scalar, const Vector<double, N>& center = Vector<double, N>::zero())
  {
    Affinity out(center);
    out *= scalar;
    return out;
  }

  /**
   * @brief Create a rotation by an angle given in radians.
   */
  static Affinity
  rotation_rad(double angle, Index from = 0, Index to = 1, const Vector<double, N>& center = Vector<double, N>::zero())
  {
    Affinity out(center);
    out.rotate_rad(angle, from, to);
    return out;
  }

  /**
   * @brief Create a rotation by an angle given in degrees.
   */
  static Affinity
  rotation_deg(double angle, Index from = 0, Index to = 1, const Vector<double, N>& center = Vector<double, N>::zero())
  {
    Affinity out(center);
    out.rotate_deg(angle, from, to);
    return out;
  }

  /**
   * @brief Translate by a given value along all axes.
   */
  Affinity& operator+=(double scalar)
  {
    m_translation += scalar;
    return *this;
  }

  /**
   * @brief Translate by a given vector.
   */
  Affinity& operator+=(const Vector<double, N>& vector)
  {
    if (not vector.is_zero()) {
      m_translation += to_eigen_vector(vector);
    }
    return *this;
  }

  /**
   * @brief Translate by the opposite of a given value along all axes.
   */
  Affinity& operator-=(double scalar)
  {
    m_translation -= scalar;
    return *this;
  }

  /**
   * @brief Translate by a the opposite of a given vector.
   */
  Affinity& operator-=(const Vector<double, N>& vector)
  {
    if (not vector.is_zero()) {
      m_translation -= to_eigen_vector(vector);
    }
    return *this;
  }

  /**
   * @brief Scale isotropically by a given factor.
   */
  Affinity& operator*=(double value)
  {
    m_map *= EigenVector::Constant(m_translation.size(), value).asDiagonal();
    return *this;
  }

  /**
   * @brief Scale by a given vector of factors.
   */
  Affinity& operator*=(const Vector<double, N>& vector)
  {
    if (not vector.is_one()) {
      m_map *= to_eigen_vector(vector).asDiagonal();
    }
    return *this;
  }

  /**
   * @brief Scale by a the inverse of given factor along all axes.
   */
  Affinity& operator/=(double value)
  {
    return operator*=(1. / value);
  }

  /**
   * @brief Scale by the inverse of a given vector of factors.
   */
  Affinity& operator/=(const Vector<double, N>& vector)
  {
    if (not vector.is_one()) {
      m_map *= to_eigen_vector(vector).cwiseInverse().asDiagonal();
    }
    return *this;
  }

  /**
   * @brief Rotate by an angle given in radians from a given axis to a given axis.
   */
  Affinity& rotate_rad(double angle, Index from = 0, Index to = 1)
  {
    if (angle != 0) {
      EigenMatrix rotation = EigenMatrix::Identity();
      const auto sin = std::sin(angle);
      const auto cos = std::cos(angle);
      rotation(from, from) = cos;
      rotation(from, to) = -sin;
      rotation(to, from) = sin;
      rotation(to, to) = cos;
      m_map *= rotation;
    }
    return *this;
  }

  /**
   * @brief Rotate by an angle given in degrees from a given axis to a given axis.
   */
  Affinity& rotate_deg(double angle, Index from = 0, Index to = 1)
  {
    return rotate_rad(Linx::pi<double>() / 180. * angle, from, to);
  }

  /**
   * @brief Inverse the transform.
   */
  Affinity& inverse()
  {
    m_map = m_map.inverse().eval();
    m_translation = -m_map * m_translation;
    return *this;
  }

  /**
   * @brief Apply the transform to an input vector.
   */
  template <typename T>
  Vector<double, N> operator()(const Vector<T, N>& in) const
  {
    return Vector<double, N>(m_translation + m_center + m_map * (to_eigen_vector(in) - m_center));
    // TODO faster without cast, i.e. without Eigen?
  }

  /**
   * @brief Apply the transform to an input interpolator.
   * 
   * The output raster has the same shape as the input (which can be a patch).
   */
  template <typename TIn>
  [[deprecated]] Raster<typename TIn::Value, TIn::Dimension> operator*(const TIn& in) const
  {
    Raster<typename TIn::Value, TIn::Dimension> out(in.shape());
    transform(in, out);
    return out;
  }

  /**
   * @brief Apply the transform with a given interpolation method.
   */
  template <typename TInterpolation, typename TIn, typename... TArgs>
  Raster<typename TIn::Value, TIn::Dimension> warp(const TIn& in, TArgs&&... args) const
  {
    Raster<typename TIn::Value, TIn::Dimension> out(in.shape());
    transform(interpolation<TInterpolation>(in, LINX_FORWARD(args)...), out);
    return out;
  }

  /**
   * @brief Apply the transform to an input interpolator.
   * 
   * The domain of the output parameter (which can be a raster or a patch)
   * is used to decide which positions to take into account.
   * If positions outside the input domain are required, then `in` must be an extrapolator, too.
   */
  template <typename TIn, typename TOut>
  TOut& transform(const TIn& in, TOut& out) const
  {
    const Affinity inv = Linx::inverse(*this);
    auto it = out.begin();
    for (const auto& p : out.domain()) {
      *it = in(inv(p));
      ++it;
    }
    return out;
  }

private:

  /**
   * @brief Copy a vector into an `EigenVector`.
   */
  template <typename T>
  static EigenVector to_eigen_vector(const Vector<T, N>& in)
  {
    EigenVector out(in.size());
    std::copy(in.begin(), in.end(), out.begin());
    return out;
  }

  /**
   * @brief The linear map.
   */
  EigenMatrix m_map;

  /**
   * @brief The translation vector.
   */
  EigenVector m_translation;

  /**
   * @brief The linear map center.
   */
  EigenVector m_center;
};

/**
 * @relatesalso Affinity
 * @brief Create the inverse transform of a given affinity.
 */
template <Index N>
Affinity<N> inverse(const Affinity<N>& in)
{
  auto out = in;
  out.inverse();
  return out;
}

/**
 * @relatesalso Affinity
 * @brief Translate an input interpolator.
 */
template <typename TInterpolation, typename TIn>
Raster<typename TIn::Value, TIn::Dimension> translate(const TIn& in, const Vector<double, TIn::Dimension>& vector)
{
  return Affinity<TIn::Dimension>::translate(vector).template warp<TInterpolation>(in); // FIXME optimize
}

/**
 * @relatesalso Affinity
 * @brief Scale an input interpolator from its center.
 */
template <typename TInterpolation, typename TIn>
Raster<typename TIn::Value, TIn::Dimension> scale(const TIn& in, double factor)
{
  const auto& domain = in.domain();
  Vector<double, TIn::Dimension> sum(domain.front() + domain.back());
  return Affinity<TIn::Dimension>::scaling(factor, sum / 2).template warp<TInterpolation>(in); // FIXME optimize
}

/**
 * @relatesalso Affinity
 * @brief Upsample an input interpolator.
 */
template <typename TInterpolation, typename TIn>
Raster<typename TIn::Value, TIn::Dimension> upsample(const TIn& in, double factor)
{
  Raster<typename TIn::Value, TIn::Dimension> out(in.shape() * factor);
  const auto scaling = Affinity<TIn::Dimension>::scaling(factor);
  scaling.transform(interpolation<TInterpolation>(in), out);
  return out;
}

/**
 * @relatesalso Affinity
 * @brief Downsample an input interpolator.
 */
template <typename TInterpolation, typename TIn>
Raster<typename TIn::Value, TIn::Dimension> downsample(const TIn& in, double factor)
{
  return upsample<TInterpolation>(in, 1. / factor);
}

/**
 * @relatesalso Affinity
 * @brief Rotate an input interpolator around its center.
 */
template <typename TInterpolation, typename TIn>
Raster<typename TIn::Value, TIn::Dimension> rotate_rad(const TIn& in, double angle, Index from = 0, Index to = 1)
{
  const auto& domain = in.domain();
  Vector<double, TIn::Dimension> sum(domain.front() + domain.back());
  return Affinity<TIn::Dimension>::rotation_rad(angle, from, to, sum / 2).template warp<TInterpolation>(in);
}

/**
 * @relatesalso Affinity
 * @brief Rotate an input interpolator around its center.
 */
template <typename TInterpolation, typename TIn>
Raster<typename TIn::Value, TIn::Dimension> rotate_deg(const TIn& in, double angle, Index from = 0, Index to = 1)
{
  const auto& domain = in.domain();
  Vector<double, TIn::Dimension> sum(domain.front() + domain.back());
  return Affinity<TIn::Dimension>::rotation_deg(angle, from, to, sum / 2).template warp<TInterpolation>(in);
}

} // namespace Linx

#endif
