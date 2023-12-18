// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_EXTRAPOLATION_H
#define _LINXTRANSFORMS_EXTRAPOLATION_H

#include "Linx/Data/Raster.h"
#include "Linx/Transforms/impl/InterpolationMethods.h"

namespace Linx {

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
class Extrapolation {
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
  explicit Extrapolation(const TRaster& raster, TMethod&& method = TMethod()) :
      m_raster(raster), m_method(std::move(method))
  {}

  /**
   * @brief Get the decorated raster.
   */
  const TRaster& raster() const
  {
    return m_raster;
  }

  /**
   * @brief Get the extrapolation method.
   */
  const TMethod& method() const
  {
    return m_method;
  }

  /**
   * @brief Get the raster shape.
   */
  const Position<Dimension>& shape() const
  {
    return m_raster.shape();
  }

  /**
   * @brief Get the raster domain.
   */
  Box<Dimension> domain() const
  {
    return m_raster.domain();
  }

  /**
   * @brief Access the element at given position.
   * 
   * If the position is outside the image domain, apply the extrapolation method.
   */
  inline const Value& operator[](const Position<Dimension>& position) const
  {
    return m_method.at(m_raster, position);
  }

  /**
   * @brief Get a decorated patch.
   */
  template <typename TRegion>
  const Patch<const Value, const Extrapolation<TRaster, TMethod>, TRegion> patch(TRegion&& region) const
  {
    return Patch<const Value, const Extrapolation<TRaster, TMethod>, TRegion>(*this, std::forward<TRegion>(region));
  }

  /**
   * @brief Get a copy of the data in a given region.
   */
  template <typename TRegion>
  Raster<std::decay_t<Value>, Dimension> copy(TRegion&& region) const
  {
    // FIXME optimize
    return Raster<std::decay_t<Value>, Dimension>(patch(std::forward<TRegion>(region)));
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

/// @cond
namespace Internal {
template <typename T>
struct IsExtrapolatorImpl : std::false_type {};

template <typename T, typename TMethod>
struct IsExtrapolatorImpl<Extrapolation<T, TMethod>> : std::true_type {};

template <typename T, typename TParent, typename TRegion>
struct IsExtrapolatorImpl<Patch<T, TParent, TRegion>> : IsExtrapolatorImpl<std::decay_t<TParent>> {};

} // namespace Internal
/// @endcond

template <typename T>
constexpr bool is_extrapolator()
{
  return Internal::IsExtrapolatorImpl<std::decay_t<T>>::value;
}

/**
 * @relatesalso Extrapolation
 * @brief Get the raster decorated by an extrapolator.
 */
template <typename TRaster, typename TMethod>
const TRaster& rasterize(const Extrapolation<TRaster, TMethod>& in)
{
  return in.raster();
}

/**
 * @relatesalso Extrapolation
 * @brief Make an extrapolator with given extrapolation method.
 */
template <typename TMethod = NearestNeighbor, typename TRaster, typename... TArgs>
auto extrapolation(const TRaster& raster, TArgs&&... args) -> decltype(auto)
{
  return Extrapolation<TRaster, TMethod>(raster, TMethod(std::forward<TArgs>(args)...));
}

/**
 * @relatesalso Extrapolation
 * @brief Make an extrapolator with constant extrapolation value.
 */
template <typename T, Index N, typename THolder>
auto extrapolation(const Raster<T, N, THolder>& raster, T constant) -> decltype(auto)
{
  return Extrapolation<Raster<T, N, THolder>, OutOfBoundsConstant<T>>(raster, OutOfBoundsConstant<T>(constant));
}

/**
 * @relatesalso Extrapolation
 * @brief Do not extrapolate if `in` is an extrapolator or patch of an extrapolator.
 */
template <typename T, Index N, typename THolder>
Raster<T, N, THolder>& dont_extrapolate(Raster<T, N, THolder>& in)
{
  return in;
}

template <typename T, Index N, typename THolder>
const Raster<T, N, THolder>& dont_extrapolate(const Raster<T, N, THolder>& in)
{
  return in;
}

template <typename TRaster, typename TMethod>
const TRaster& dont_extrapolate(const Extrapolation<TRaster, TMethod>& in)
{
  return in.raster();
}

template <typename T, typename TParent, typename TRegion>
decltype(auto) dont_extrapolate(Patch<T, TParent, TRegion>& in)
{
  return dont_extrapolate(in.parent()).patch(in.domain()); // FIXME avoid region copy if TParent is no extrapolator
}

template <typename T, typename TParent, typename TRegion>
decltype(auto) dont_extrapolate(const Patch<T, TParent, TRegion>& in)
{
  return dont_extrapolate(in.parent()).patch(in.domain()); // FIXME avoid region copy if TParent is no extrapolator
}

} // namespace Linx

#endif
