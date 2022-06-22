// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERGEOMETRY_INTERPOLATION_POLICIES_H
#define _RASTERGEOMETRY_INTERPOLATION_POLICIES_H

#include "LitlRaster/Raster.h"

namespace Litl {

/**
 * @ingroup interpolation
 * @brief Constant, a.k.a. Dirichlet boundary conditions.
 */
template <typename T>
struct OutOfBoundsConstant {

  /**
   * @brief Constructor.
   */
  OutOfBoundsConstant(T v = T {}) : value(v) {}

  /**
   * @brief Return `value` if out of bounds.
   */
  template <typename TRaster>
  inline const T& at(TRaster& raster, const Position<TRaster::Dimension>& position) const {
    if (raster.contains(position)) {
      return raster[position];
    }
    return value;
  }

  /**
   * @brief The extrapolation value.
   */
  T value;
};

/**
 * @ingroup interpolation
 * @brief Nearest-neighbor interpolation or extrapolation, a.k.a. zero-flux Neumann boundary conditions.
 */
struct NearestNeighbor {

  /**
   * @brief Return the value at the nearest in-bounds position.
   */
  template <typename TRaster>
  inline const typename TRaster::value_type& at(TRaster& raster, const Position<TRaster::Dimension>& position) const {
    auto in = clamp(position, raster.shape());
    return raster[in];
  }

  /**
   * @brief Return the value at the nearest integer position.
   */
  template <typename T, typename TRaster>
  inline T at(TRaster& raster, const Vector<double, TRaster::Dimension>& position) const {
    Position<TRaster::Dimension> integral(position.size());
    std::transform(position.begin(), position.end(), integral.begin(), [](auto e) {
      return e + .5;
    });
    return raster[integral];
  }
};

/**
 * @ingroup interpolation
 * @brief Periodic, a.k.a. symmetric or wrap-around, boundary conditions.
 */
struct Periodic {

  /**
   * @brief Return the value at the modulo position.
   */
  template <typename TRaster>
  inline const typename TRaster::value_type& at(TRaster& raster, const Position<TRaster::Dimension>& position) const {
    Position<TRaster::Dimension> inbounds(position.size());
    inbounds.generate(
        [](auto p, auto s) {
          auto q = p % s;
          return q < 0 ? q + s : q; // Positive modulo
        },
        position,
        raster.shape());
    return raster[inbounds];
  }
};

/**
 * @ingroup interpolation
 * @brief Linear interpolation.
 */
struct Linear {

  template <typename T, typename TRaster, typename... TIndices>
  inline T at(const TRaster& raster, const Vector<double, 1>& position, TIndices... indices) const {
    const auto f = floor<Index>(position.front());
    const auto d = position.front() - f;
    const T p = raster[{f, indices...}];
    const T n = raster[{f + 1, indices...}];
    return d * (p - n) + n;
  }

  template <typename T, Index N, typename TRaster, typename... TIndices>
  inline std::enable_if_t<N != 1, T>
  at(const TRaster& raster, const Vector<double, N>& position, TIndices... indices) const {
    const auto f = floor<Index>(position.back());
    const auto d = position.back() - f;
    const auto pos = position.template slice<N - 1>();
    const auto p = at<T>(raster, pos, f, indices...);
    const auto n = at<T>(raster, pos, f + 1, indices...);
    return d * (p - n) + n;
  }
};

/**
 * @ingroup interpolation
 * @brief Cubic interpolation.
 */
struct Cubic {

  template <typename T, typename TRaster, typename... TIndices>
  inline T at(const TRaster& raster, const Vector<double, 1>& position, TIndices... indices) const {
    const auto f = floor<Index>(position.front());
    const auto d = position.front() - f;
    const T pp = raster[{f - 1, indices...}];
    const T p = raster[{f, indices...}];
    const T n = raster[{f + 1, indices...}];
    const T nn = raster[{f + 2, indices...}];
    return p + 0.5 * (d * (-pp + n) + d * d * (2 * pp - 5 * p + 4 * n - nn) + d * d * d * (-pp + 3 * p - 3 * n + nn));
  }

  template <typename T, Index N, typename TRaster, typename... TIndices>
  inline std::enable_if_t<N != 1, T>
  at(const TRaster& raster, const Vector<double, N>& position, TIndices... indices) const {
    const auto f = floor<Index>(position.back());
    const auto d = position.back() - f;
    const auto pos = position.template slice<N - 1>();
    const auto pp = at<T>(raster, pos, f - 1, indices...);
    const auto p = at<T>(raster, pos, f, indices...);
    const auto n = at<T>(raster, pos, f + 1, indices...);
    const auto nn = at<T>(raster, pos, f + 2, indices...);
    return p + 0.5 * (d * (-pp + n) + d * d * (2 * pp - 5 * p + 4 * n - nn) + d * d * d * (-pp + 3 * p - 3 * n + nn));
  }
};

} // namespace Litl

#endif
