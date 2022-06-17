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
  using Value = std::decay_t<T>;

  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dim = N;

  /**
   * @brief Constructor.
   */
  template <typename TRaster>
  Extrapolator(const TRaster& raster, TPolicy&& policy) :
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
 * @brief Interpolator with optional extrapolator.
 * @tparam TPolicy The interpolation policy
 * @tparam TRaster The raster or extrapolator type
 * @details
 * This class provides a subscript operator which accepts a position,
 * which can lie between pixels or outside the raster domain.
 * 
 * The interpolation formula is implemented as `TPolicy::at()`,
 * while the extrapolation formula is implemented as `TRaster::operator[]()`.
 * If `TRaster` is a raster, then no bound checking is performed.
 * This is the best option when no value outside the raster domain has to be evaluated.
 */
template <typename TPolicy, typename TRaster>
class Interpolator {

public:
  /**
   * @brief The interpolation value type.
   */
  using Value = typename InterpolationTraits<typename TRaster::Value>::Value;

  /**
   * @brief The interpolation dimension parameter.
   */
  static constexpr Index Dim = TRaster::Dim;

  /**
   * @brief Constructor.
   */
  Interpolator(const TRaster& raster, TPolicy&& policy) : m_policy(std::move(policy)), m_raster(raster) {}

  /**
   * @brief Compute the value at given integral position.
   */
  inline const typename TRaster::Value& operator[](const Position<Dim>& position) const {
    return m_raster[position];
  }

  /**
   * @brief Compute the interpolated value at given position.
   */
  inline Value operator[](const Vector<double, Dim>& position) const {
    return m_policy.template at<Value>(m_raster, position);
  }

private:
  /**
   * @brief The interpolation policy.
   */
  TPolicy m_policy;

  /**
   * @brief The raster or extrapolator.
   */
  const TRaster& m_raster;
};

template <typename TPolicy, typename TRaster, typename... TArgs>
auto extrapolate(const TRaster& raster, TArgs&&... args) -> decltype(auto) {
  return Extrapolator<TPolicy, typename TRaster::Value, TRaster::Dim>(raster, TPolicy(std::forward<TArgs>(args)...));
}

template <typename T, Index N, typename THolder>
auto extrapolate(const Raster<T, N, THolder>& raster, T constant) -> decltype(auto) {
  return Extrapolator<OutOfBoundsConstant<T>, T, N>(raster, OutOfBoundsConstant<T>(constant));
}

template <typename TPolicy = NearestNeighbor, typename TRaster, typename... TArgs>
auto interpolate(const TRaster& raster, TArgs&&... args) -> decltype(auto) {
  return Interpolator<TPolicy, TRaster>(raster, TPolicy(std::forward<TArgs>(args)...));
}

} // namespace Cnes

#endif
