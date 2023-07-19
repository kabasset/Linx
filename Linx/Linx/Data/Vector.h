// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXDATA_VECTOR_H
#define _LINXDATA_VECTOR_H

#include "Linx/Base/DataContainer.h"
#include "Linx/Base/Math.h" // abspow
#include "Linx/Base/TypeUtils.h"

#include <numeric> // accumulate, multiplies
#include <type_traits> // conditional

namespace Linx {

/**
 * @brief A container of coordinates.
 */
template <typename T, Index N = 2>
using Coordinates = typename std::conditional<(N == -1), std::vector<T>, std::array<T, (std::size_t)N>>::type;

/**
 * @relatesalso Position
 * @brief The index container type.
 */
template <Index N = 2>
using Indices = Coordinates<Index, N>;

/**
 * @ingroup data_classes
 * @brief N-dimensional vector, mainly intended for pixel position or image shape, i.e. set of coordinates.
 * @tparam N A non-negative dimension (0 is allowed), or -1 for variable dimension.
 * 
 * The values are stored in a `std::array<T, N>` in general (`N &ge; 0`),
 * or `std::vector<T>` for variable dimension (`N = -1`).
 *
 * Memory and services are optimized when dimension is fixed at compile-time (`N &ge; 0`).
 * 
 * @tspecialization{Position}
 */
template <typename T, Index N = 2>
class Vector : public DataContainer<T, StdHolder<Coordinates<T, N>>, VectorArithmetic, Vector<T, N>> {
public:
  /**
   * @brief The value type.
   */
  using Value = T;

  /**
   * @brief The dimension template parameter.
   */
  static constexpr Index Dimension = N;

  /**
   * @brief The container type.
   */
  using Container = DataContainer<T, StdHolder<Coordinates<T, N>>, VectorArithmetic, Vector<T, N>>;

  LINX_VIRTUAL_DTOR(Vector)
  LINX_DEFAULT_COPYABLE(Vector)
  LINX_DEFAULT_MOVABLE(Vector)

  /**
   * @brief Default constructor.
   * @warning
   * The values are unspecified.
   * To create vector 0, use `zero()` instead.
   */
  Vector() : Container() {}

  /**
   * @brief Create a vector of given dimension.
   */
  template <typename TInt, typename std::enable_if_t<std::is_integral<TInt>::value>* = nullptr>
  explicit Vector(TInt dim) : Container(std::size_t(dim)) {}

  /**
   * @brief Create a vector from a brace-enclosed list of indices.
   */
  Vector(std::initializer_list<T> indices) : Container(indices.begin(), indices.end()) {}

  /**
   * @brief Create a vector from a range.
   */
  template <typename TRange, typename std::enable_if_t<IsRange<TRange>::value>* = nullptr>
  explicit Vector(TRange&& range) : Container(range.begin(), range.end()) {}

  /**
   * @brief Create a vector full of `Limits::zero()'s.
   */
  static Vector<T, N> zero() {
    Vector<T, N> res(std::abs(N));
    return res.fill(Limits<T>::zero());
  }

  /**
   * @brief Create a vector full of `Limits::one()'s.
   */
  static Vector<T, N> one() {
    Vector<T, N> res(std::abs(N));
    return res.fill(Limits<T>::one());
  }

  /**
   * @brief Create a vector full of `Limits::inf()'s.
   */
  static Vector<T, N> inf() {
    Vector<T, N> res(std::abs(N));
    return res.fill(Limits<T>::inf());
  }

  /**
   * @brief Check whether the vector is zero.
   */
  bool isZero() const {
    for (auto i : *this) {
      if (i != Limits<T>::zero()) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Check whether the vector is one.
   */
  bool isOne() const {
    for (auto i : *this) {
      if (i != Limits<T>::one()) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Check whether the vector is minus one.
   */
  bool isInf() const {
    for (auto i : *this) {
      if (i != Limits<T>::inf()) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Create a vector of lower dimension.
   * @tparam M The new dimension; cannot be -1
   * 
   * The values up to dimension `M` are copied.
   */
  template <Index M>
  Vector<T, M> slice() const { // FIXME free function
    Vector<T, M> res(M);
    std::copy_n(this->data(), M, res.data());
    return res;
  }

  /**
   * @brief Create a vector of higher dimension.
   * @tparam M The new dimension; cannot be -1
   * 
   * The values up to dimension `N` are copied.
   * Those between dimensions `N` and `M` are taken from the given padding vector.
   */
  template <Index M>
  Vector<T, M> extend(const Vector<T, M>& padding = Vector<T, M>::zero()) const { // FIXME free function
    auto res = padding;
    std::copy_n(this->data(), this->size(), res.data());
    return res;
  }
};

/**
 * @relatesalso Vector
 * @brief Erase an element from a given vector.
 * 
 * The size of the resulting vector is that of the input vector minus one.
 */
template <Index I, typename T, Index N>
Vector<T, N == -1 ? -1 : N - 1> erase(const Vector<T, N>& in) {
  static_assert(I >= 0);
  constexpr auto M = N == -1 ? -1 : N - 1;
  Vector<T, M> out(in.size() - 1);
  for (std::size_t i = 0; i < I; ++i) {
    out[i] = in[i];
  }
  for (std::size_t i = I; i < out.size(); ++i) {
    out[i] = in[i + 1];
  }
  return out;
}

/**
 * @relatesalso Vector
 * @brief Insert an element into a given vector.
 * 
 * The size of the resulting vector is that of the input vector plus one.
 */
template <Index I, typename T, Index N>
Vector<T, N == -1 ? -1 : N + 1> insert(const Vector<T, N>& in, T&& value) {
  static_assert(I >= 0);
  constexpr auto M = N == -1 ? -1 : N + 1;
  Vector<T, M> out(in.size() + 1);
  for (std::size_t i = 0; i < I; ++i) {
    out[i] = in[i];
  }
  out[I] = std::forward<T>(value);
  for (std::size_t i = I + 1; i < out.size(); ++i) {
    out[i] = in[i - 1];
  }
  return out;
}

/**
 * @ingroup data_classes
 * @brief Pixel position or image shape, i.e. set of indices.
 * @tparam N A non-negative dimension (0 is allowed), or -1 for variable dimension.
 * 
 * Anonymous brace-initialization is permitted, e.g.:
 * \code
 * Raster<float> raster({1920, 1080});
 * // Is equivalent to
 * Raster<float> raster(Position<2>({1920, 1080}));
 * \endcode
 * 
 * Classical positions are instantiated with named constructors, e.g.:
 * \code
 * auto bottomLeft = Position<2>::zero();
 * auto topRight = Position<2>::inf();
 * \endcode
 * 
 * @see Box
 */
template <Index N = 2>
using Position = Vector<Index, N>;

/**
 * @relatesalso Position
 * @brief Get the stride along a given axis.
 */
template <Index N>
Index shapeStride(const Position<N>& shape, Index axis) {
  return std::accumulate(shape.begin(), shape.begin() + axis, 1L, [](auto s, auto l) {
    return l > 0 ? s * l : 0; // FIXME here or for shapeSize only?
  });
}

/**
 * @relatesalso Position
 * @brief Get the stride along a given axis.
 */
template <Index Axis, Index N>
Index shapeStride(const Position<N>& shape) {
  return shapeStride(shape, Axis);
}

/**
 * @relatesalso Position
 * @brief Compute the number of pixels in a given shape.
 */
template <Index N = 2>
Index shapeSize(const Position<N>& shape) {
  const auto size = shape.size();
  if (size == 0) {
    return 0;
  }
  return shapeStride(shape, size);
}

/**
 * @brief Compute the Lp-norm of a vector raised to the power p.
 * @tparam P The power
 */
template <Index P, typename T, Index N>
T norm(const Vector<T, N>& in) {
  T out(0);
  for (const auto& e : in) {
    out += abspow<P>(e);
  }
  return out;
}

/**
 * @brief Compute the absolute Lp-distance between two vectors raised to the power p.
 * @tparam P The power
 */
template <Index P, typename T, Index N>
T distance(const Vector<T, N>& lhs, const Vector<T, N>& rhs) {
  return std::inner_product(lhs.begin(), lhs.end(), rhs.begin(), 0., std::plus<T> {}, [](T a, T b) {
    return abspow<P>(b - a);
  });
}

} // namespace Linx

#endif
