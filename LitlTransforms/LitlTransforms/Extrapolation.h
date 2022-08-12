// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLTRANSFORMS_EXTRAPOLATION_H
#define _LITLTRANSFORMS_EXTRAPOLATION_H

#include "LitlRaster/Raster.h"
#include "LitlTransforms/impl/InterpolationMethods.h"

namespace Litl {

/**
 * @ingroup interpolation
 * @brief Extrapolation decorator.
 * @tparam TRaster The decorated raster type
 * @tparam TMethod The extrapolation method
 * 
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
  using Value = const typename TRaster::Value;

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
   * @brief Get the extrapolation method.
   */
  const TMethod& method() const {
    return m_method;
  }

  /**
   * @brief Get a decorated subraster.
   */
  template <typename TRegion>
  const Subraster<const Value, const Extrapolator<TRaster, TMethod>, TRegion> subraster(TRegion&& region) const {
    return Subraster<const Value, const Extrapolator<TRaster, TMethod>, TRegion>(*this, std::forward<TRegion>(region));
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
   * 
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
 * @relates Extrapolator
 * @brief Get the raster decorated by an extrapolator.
 */
template <typename TRaster, typename TMethod>
const TRaster& rasterize(const Extrapolator<TRaster, TMethod>& in) {
  return in.raster();
}

/**
 * @relates Extrapolator
 * @brief Make an extrapolator with given extrapolation method.
 */
template <typename TMethod = NearestNeighbor, typename TRaster, typename... TArgs>
auto extrapolate(const TRaster& raster, TArgs&&... args) -> decltype(auto) {
  return Extrapolator<TRaster, TMethod>(raster, TMethod(std::forward<TArgs>(args)...));
}

/**
 * @relates Extrapolator
 * @brief Make an extrapolator with constant extrapolation value.
 */
template <typename T, Index N, typename THolder>
auto extrapolate(const Raster<T, N, THolder>& raster, T constant) -> decltype(auto) {
  return Extrapolator<Raster<T, N, THolder>, OutOfBoundsConstant<T>>(raster, OutOfBoundsConstant<T>(constant));
}

/**
 * @relates Extrapolator
 * @brief Do not extrapolate if `in` is an extrapolator or subraster of an extrapolator.
 */
template <typename T, Index N, typename THolder>
Raster<T, N, THolder>& dontExtrapolate(Raster<T, N, THolder>& in) {
  return in;
}

template <typename T, Index N, typename THolder>
const Raster<T, N, THolder>& dontExtrapolate(const Raster<T, N, THolder>& in) {
  return in;
}

template <typename TRaster, typename TMethod>
const TRaster& dontExtrapolate(const Extrapolator<TRaster, TMethod>& in) {
  return in.raster();
}

template <typename T, typename TParent, typename TRegion>
auto dontExtrapolate(Subraster<T, TParent, TRegion>& in) -> decltype(auto) {
  return dontExtrapolate(in.parent()).subraster(in.region()); // FIXME avoid region copy if TParent is no extrapolator
}

template <typename T, typename TParent, typename TRegion>
auto dontExtrapolate(const Subraster<T, TParent, TRegion>& in) -> decltype(auto) {
  return dontExtrapolate(in.parent()).subraster(in.region()); // FIXME avoid region copy if TParent is no extrapolator
}

} // namespace Litl

#endif
