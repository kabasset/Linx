// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_INTERPOLATION_H
#define _LITLTRANSFORMS_INTERPOLATION_H

#include "LitlRaster/Raster.h"
#include "LitlTransforms/impl/InterpolationMethods.h"

namespace Litl {

/**
 * @ingroup interpolation
 * @brief Interpolation decorator with optional extrapolator.
 * @tparam TRaster The decorated raster or extrapolator type
 * @tparam TMethod The interpolation method
 * 
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
   * @brief Get the interpolation method.
   */
  const TMethod& method() const {
    return m_method;
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

/**
 * @relates Interpolator
 * @brief Make an interpolator with given interpolation method.
 */
template <typename TMethod = NearestNeighbor, typename TRaster, typename... TArgs>
auto interpolate(const TRaster& raster, TArgs&&... args) -> decltype(auto) {
  return Interpolator<TRaster, TMethod>(raster, TMethod(std::forward<TArgs>(args)...));
}

} // namespace Litl

#endif
