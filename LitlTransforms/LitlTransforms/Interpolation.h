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
 * @tparam TMethod The extrapolation method
 * @details
 * This class provides a subscript operator which accepts an integral position
 * which can lie outside the raster domain.
 * 
 * The the extrapolation formula is implemented as `TMethod::at()`.
 */
template <typename TMethod, typename T, Index N>
class Extrapolator {

public:
  /**
   * @brief The value type.
   */
  using Value = std::decay_t<T>;

  /**
   * @brief The dimension parameter.
   */
  static constexpr Index Dimension = N;

  /**
   * @brief Constructor.
   */
  template <typename TRaster>
  Extrapolator(const TRaster& raster, TMethod&& method) :
      m_method(std::move(method)), m_raster(raster.shape(), raster.data()) {}

  /**
   * @brief Get the raster shape.
   */
  Position<N> shape() const {
    return m_raster.shape();
  }

  /**
   * @brief Get the raster domain.
   */
  Region<N> domain() const {
    return m_raster.domain();
  }

  /**
   * @brief Access the element at given in-bounds position.
   */
  inline const T& operator[](const Position<N>& position) const {
    return m_raster[position];
  }

  /**
   * @brief Access the element at given position.
   * @details
   * If the position is outside the image domain, apply the extrapolation method.
   */
  inline const T& at(const Position<N>& position) const {
    return m_method.at(m_raster, position);
  }

private:
  /**
   * @brief The extrapolation method.
   */
  TMethod m_method;

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
 * @tparam TMethod The interpolation method
 * @tparam TRaster The raster or extrapolator type
 * @details
 * This class provides a subscript operator which accepts a position,
 * which can lie between pixels or outside the raster domain.
 * 
 * The interpolation formula is implemented as `TMethod::at()`,
 * while the extrapolation formula is implemented as `TRaster::operator[]()`.
 * If `TRaster` is a raster, then no bound checking is performed.
 * This is the best option when no value outside the raster domain has to be evaluated.
 */
template <typename TMethod, typename TRaster>
class Interpolator {

public:
  /**
   * @brief The interpolation value type.
   */
  using Value = typename InterpolationTraits<typename TRaster::Value>::Value;

  /**
   * @brief The interpolation dimension parameter.
   */
  static constexpr Index Dimension = TRaster::Dimension;

  /**
   * @brief Constructor.
   */
  Interpolator(const TRaster& raster, TMethod&& method) : m_method(std::move(method)), m_raster(raster) {}

  /**
   * @brief Get the raster shape.
   */
  Position<Dimension> shape() const {
    return m_raster.shape();
  }

  /**
   * @brief Get the raster domain.
   */
  Region<Dimension> domain() const {
    return m_raster.domain();
  }

  /**
   * @brief Compute the value at given in-bounds integral position.
   */
  inline const typename TRaster::Value& operator[](const Position<Dimension>& position) const {
    return m_raster[position];
  }

  /**
   * @brief Compute the value at given integral position.
   */
  inline const typename TRaster::Value& at(const Position<Dimension>& position) const {
    return m_raster.at(position);
  }

  /**
   * @brief Compute the interpolated value at given position.
   */
  inline Value operator()(const Vector<double, Dimension>& position) const { // FIXME naming?
    return m_method.template at<Value>(m_raster, position);
  }

private:
  /**
   * @brief The interpolation method.
   */
  TMethod m_method;

  /**
   * @brief The raster or extrapolator.
   */
  const TRaster& m_raster;
};

template <typename TMethod, typename TRaster, typename... TArgs>
auto extrapolate(const TRaster& raster, TArgs&&... args) -> decltype(auto) {
  return Extrapolator<TMethod, typename TRaster::Value, TRaster::Dimension>(
      raster,
      TMethod(std::forward<TArgs>(args)...));
}

template <typename T, Index N, typename THolder>
auto extrapolate(const Raster<T, N, THolder>& raster, T constant) -> decltype(auto) {
  return Extrapolator<OutOfBoundsConstant<T>, T, N>(raster, OutOfBoundsConstant<T>(constant));
}

template <typename TMethod = NearestNeighbor, typename TRaster, typename... TArgs>
auto interpolate(const TRaster& raster, TArgs&&... args) -> decltype(auto) {
  return Interpolator<TMethod, TRaster>(raster, TMethod(std::forward<TArgs>(args)...));
}

} // namespace Litl

#endif
