// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXTRANSFORMS_INTERPOLATION_H
#define _LINXTRANSFORMS_INTERPOLATION_H

#include "Linx/Data/Raster.h"
#include "Linx/Transforms/impl/ResamplingMethods.h"

namespace Linx {

/**
 * @ingroup resampling
 * @brief Interpolation decorator with optional extrapolator.
 * @tparam TParent The decorated raster or extrapolator type
 * @tparam TMethod The interpolation method
 * 
 * This class provides a function call operator which accepts a non-integral position.
 * If `TParent` is a raster extrapolator, then the position can be outside the raster domain.
 * 
 * The interpolation formula is implemented as `TMethod::at()`,
 * while the extrapolation formula (if any) is implemented as `TParent::operator[]()`.
 * 
 * If `TParent` is a raster, then no bound checking is performed.
 * This is the fastest option when no value outside the raster domain has to be evaluated.
 */
template <typename TParent, typename TMethod>
class Interpolation {
public:

  /**
   * @brief The input value type.
   */
  using Value = typename TParent::Value;

  /**
   * @brief The interpolation value type.
   */
  using Floating = typename TypeTraits<Value>::Floating;

  /**
   * @brief The interpolation dimension parameter.
   */
  static constexpr Index Dimension = TParent::Dimension;

  /**
   * @brief Constructor.
   */
  explicit Interpolation(const TParent& parent, TMethod&& method = TMethod()) :
      m_parent(parent), m_method(std::move(method))
  {}

  /**
   * @brief Get the decorated parent.
   */
  const TParent& parent() const
  {
    return m_parent;
  }

  /**
   * @brief Get the decorated raster.
   */
  decltype(auto) raster() const
  { // FIXME As free function?
    return Linx::rasterize(m_parent);
  }

  /**
   * @brief Get the interpolation method.
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
    return m_parent.shape();
  }

  /**
   * @brief Get the raster domain.
   */
  Box<Dimension> domain() const
  {
    return m_parent.domain();
  }

  /**
   * @brief Get the value at given integral position.
   */
  inline const Value& operator[](const Position<Dimension>& position) const
  {
    return m_parent[position];
  }

  /**
   * @brief Compute the interpolated value at given position.
   */
  inline Floating operator()(const Vector<double, Dimension>& position) const
  {
    return m_method.template at<Floating>(m_parent, position);
  }

private:

  /**
   * @brief The raster or extrapolator.
   */
  const TParent& m_parent;

  /**
   * @brief The interpolation method.
   */
  TMethod m_method;
};

/**
 * @relatesalso Interpolation
 * @brief Make an interpolator with given interpolation method.
 */
template <typename TMethod = Nearest, typename TParent, typename... TArgs>
decltype(auto) interpolation(const TParent& parent, TArgs&&... args)
{
  return Interpolation<TParent, TMethod>(parent, TMethod(std::forward<TArgs>(args)...));
}

} // namespace Linx

#endif
