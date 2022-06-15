// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERGEOMETRY_INTERPOLATION_H
#define _RASTERGEOMETRY_INTERPOLATION_H

#include "Raster/Raster.h"
#include "RasterGeometry/InterpolationPolicies.h"

namespace Cnes {

/**
 * @ingroup interpolation
 * @brief Extrapolation decorator.
 * @tparam TPolicy The extrapolation policy
 * @details
 * This class provides a subscript operator which accepts an integral position
 * which can lie outside the raster domain.
 * 
 * The the extrapolation formula is implemented as `TPolicy::at()`.
 */
template <typename TPolicy, typename T, Index N>
class Extrapolator {

public:
  /**
   * @brief The value type.
   */
  using Value = T;

  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dim = N;

  /**
   * @brief Constructor.
   */
  template <typename TRaster>
  Extrapolator(TPolicy&& policy, const TRaster& raster) :
      m_policy(std::move(policy)), m_raster(raster.shape(), raster.data()) {}

  /**
   * @brief Access the element at given position.
   * @details
   * If the position is outside the image domain, rely on the extrapolation policy.
   */
  inline const T& operator[](const Position<N>& position) const {
    return m_policy.at(m_raster, position);
  }

private:
  /**
   * @brief The extrapolation policy.
   */
  TPolicy m_policy;

  /**
   * @brief The input raster.
   */
  PtrRaster<const T, N> m_raster;
};

template <typename T>
struct InterpolationTraits {
  using Value = double;
};

template <typename T>
struct InterpolationTraits<std::complex<T>> {
  using Value = std::complex<double>;
};

/**
 * @ingroup interpolation
 * @brief Interpolation decorator.
 * @tparam TPolicy The interpolation policy
 * @tparam TExtrapolator The extrapolator, which can be a mere `Raster`
 * @details
 * This class provides a subscript operator which accepts a position,
 * which can lie between pixels or outside the raster domain.
 * 
 * The interpolation formula is implemented as `TPolicy::at()`,
 * while the extrapolation formula is implemented as `TExtrapolator::operator[]()`.
 * If `TExtrapolator` is a raster, then no bound checking is performed.
 * This is the best option when no value outside the raster domain has to be evaluated.
 */
template <typename TPolicy, typename TExtrapolator>
class Interpolator {

public:
  /**
   * @brief The interpolation value type.
   */
  using Value = typename InterpolationTraits<typename TExtrapolator::Value>::Value;

  /**
   * @brief The interpolation dimension parameter.
   */
  static constexpr Index Dim = TExtrapolator::Dim;

  /**
   * @brief Constructor.
   */
  Interpolator(TPolicy&& policy, const TExtrapolator& extrapolator) :
      m_policy(std::move(policy)), m_extrapolator(std::move(extrapolator)) {}

  /**
   * @brief Compute the value at given integral position.
   */
  inline typename TExtrapolator::Value operator[](const Position<Dim>& position) const {
    return m_extrapolator[position];
  }

  /**
   * @brief Compute the interpolated value at given position.
   */
  inline Value operator[](const Vector<double, Dim>& position) const {
    return m_policy.template at<Value>(m_extrapolator, position);
  }

private:
  /**
   * @brief The interpolation policy.
   */
  TPolicy m_policy;

  /**
   * @brief The extrapolation decorator.
   */
  const TExtrapolator& m_extrapolator;
};

} // namespace Cnes

#endif
