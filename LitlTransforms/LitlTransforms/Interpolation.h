// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_INTERPOLATION_H
#define _LITLTRANSFORMS_INTERPOLATION_H

#include "LitlRaster/Raster.h"
#include "LitlTransforms/InterpolationMethods.h"

namespace Litl {

/**
 * @ingroup interpolation
 * @brief Extrapolation decorator.
 * @tparam TRaster The decorated raster type
 * @tparam TMethod The extrapolation method
 * @details
 * This class provides a subscript operator which accepts an integral position
 * which can lie outside the raster domain.
 * 
 * The the extrapolation formula is implemented as `TMethod::at()`.
 */
template <typename TRaster, typename TMethod>
class Extrapolator {

public:
  /**
   * @brief The value type.
   */
  using Value = typename TRaster::Value;

  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = TRaster::Dimension;

  /**
   * @brief Constructor.
   */
  explicit Extrapolator(const TRaster& raster, TMethod&& method = TMethod()) :
      m_raster(raster), m_method(std::move(method)) {}

  /**
   * @brief Get the decorated raster.
   */
  const TRaster& raster() const {
    return m_raster;
  }

  /**
   * @brief Get the raster shape.
   */
  const Position<Dimension>& shape() const {
    return m_raster.shape();
  }

  /**
   * @brief Get the raster domain.
   */
  Box<Dimension> domain() const {
    return m_raster.domain();
  }

  /**
   * @brief Access the element at given position.
   * @details
   * If the position is outside the image domain, apply the extrapolation method.
   */
  inline const Value& operator[](const Position<Dimension>& position) const {
    return m_method.at(m_raster, position);
  }

private:
  /**
   * @brief The input raster.
   */
  const TRaster& m_raster;

  /**
   * @brief The extrapolation method.
   */
  TMethod m_method;
};

/**
 * @brief Identity.
 * @details
 * This function is provided for compatibility with `Extrapolator`
 * in cases where functions accept either a `Raster` or an `Extrapolator`.
 */
template <typename T, Index N, typename THolder>
const Raster<T, N, THolder>& rasterize(const Raster<T, N, THolder>& in) {
  return in;
}

/**
 * @brief Get the raster decorated by an extrapolator.
 */
template <typename TRaster, typename TMethod>
const TRaster& rasterize(const Extrapolator<TRaster, TMethod>& in) {
  return in.raster();
}

/**
 * @ingroup interpolation
 * @brief Interpolation decorator with optional extrapolator.
 * @tparam TRaster The decorated raster or extrapolator type
 * @tparam TMethod The interpolation method
 * @details
 * This class provides a subscript operator which accepts a position,
 * which can lie between pixels or outside the raster domain.
 * 
 * The interpolation formula is implemented as `TMethod::at()`,
 * while the extrapolation formula is implemented as `TRaster::operator[]()`.
 * If `TRaster` is a raster, then no bound checking is performed.
 * This is the best option when no value outside the raster domain has to be evaluated.
 */
template <typename TRaster, typename TMethod>
class Interpolator {

public:
  /**
   * @brief The input value type.
   */
  using Value = typename TRaster::Value;

  /**
   * @brief The interpolation value type.
   */
  using Floating = typename TypeTraits<Value>::Floating;

  /**
   * @brief The interpolation dimension parameter.
   */
  static constexpr Index Dimension = TRaster::Dimension;

  /**
   * @brief Constructor.
   */
  explicit Interpolator(const TRaster& raster, TMethod&& method = TMethod()) :
      m_raster(raster), m_method(std::move(method)) {}

  /**
   * @brief Get the decorated raster.
   */
  auto raster() const -> decltype(auto) {
    return Litl::rasterize(m_raster);
  }

  /**
   * @brief Get the raster shape.
   */
  const Position<Dimension>& shape() const {
    return m_raster.shape();
  }

  /**
   * @brief Get the raster domain.
   */
  Box<Dimension> domain() const {
    return m_raster.domain();
  }

  /**
   * @brief Get the value at given integral position.
   */
  inline const Value& operator[](const Position<Dimension>& position) const {
    return m_raster[position];
  }

  /**
   * @brief Compute the interpolated value at given position.
   */
  inline Floating operator()(const Vector<double, Dimension>& position) const {
    return m_method.template at<Floating>(m_raster, position);
  }

private:
  /**
   * @brief The raster or extrapolator.
   */
  const TRaster& m_raster;

  /**
   * @brief The interpolation method.
   */
  TMethod m_method;
};

template <typename TMethod = NearestNeighbor, typename TRaster, typename... TArgs>
auto extrapolate(const TRaster& raster, TArgs&&... args) -> decltype(auto) {
  return Extrapolator<TRaster, TMethod>(raster, TMethod(std::forward<TArgs>(args)...));
}

template <typename T, Index N, typename THolder>
auto extrapolate(const Raster<T, N, THolder>& raster, T constant) -> decltype(auto) {
  return Extrapolator<Raster<T, N, THolder>, OutOfBoundsConstant<T>>(raster, OutOfBoundsConstant<T>(constant));
}

template <typename TMethod = NearestNeighbor, typename TRaster, typename... TArgs>
auto interpolate(const TRaster& raster, TArgs&&... args) -> decltype(auto) {
  return Interpolator<TRaster, TMethod>(raster, TMethod(std::forward<TArgs>(args)...));
}

} // namespace Litl

#endif
