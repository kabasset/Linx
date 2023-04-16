// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXCORE_RASTER_H
#define _LINXCORE_RASTER_H

#include "LinxBase/AlignedBuffer.h"
#include "LinxBase/DataContainer.h"
#include "LinxBase/Exceptions.h"
#include "LinxBase/Random.h"
#include "LinxCore/Box.h"

#include <complex>
#include <cstdint>
#include <string>
#include <valarray>
#include <vector>

/**
 * @brief The light image template library
 * 
 * An ND image processing library developed and maintained by the French space agency,
 * Centre National d'Etudes Spatiales.
 * @see https://github.com/cnes
 * @see https://cnes.fr/en
 */
namespace Linx {

/// @cond
// Issue with forward declarations: https://github.com/doxygen/doxygen/issues/8177

// Forward declaration for Raster::patch()
template <typename T, typename TRaster, typename TRegion>
class Patch;

// Forward declaration for specializations
template <typename T, Index N, typename THolder>
class Raster;

/// @endcond

/**
 * @ingroup data_classes
 * @brief `Raster` which points to some external data.
 * 
 * It is a non-owning container: no allocation or freeing is made; a `PtrRaster`'s memory is managed externally.
 * This is the type which is preferred to represent contiguous views, e.g. with `Raster::section()`.
 */
template <typename T, Index N = 2>
using PtrRaster = Raster<T, N, PtrHolder<T>>;

/**
 * @ingroup data_classes
 * @brief `Raster` which owns a `std::vector`.
 * 
 * The underlying container is a `std::vector`.
 * It is convenient to interface with the standard library
 * or other tools which work with `std::vector`s without copies,
 * because `std::vector` itself cannot be initialized without copies from an external pointer.
 * @warning
 * Because specialization `std::vector<bool>` is not a container,
 * `VecRaster` is not compatible with `bool` values.
 * This can be worked around with values of type `char`,
 * or another holder has to be used.
 */
template <typename T, Index N = 2, typename TAllocator = std::allocator<T>>
using VecRaster = Raster<T, N, StdHolder<std::vector<T, TAllocator>>>;

/**
 * @ingroup data_classes
 * @brief `Raster` which owns a `std::valarray`.
 */
template <typename T, Index N = 2>
using ValRaster = Raster<T, N, StdHolder<std::valarray<T>>>;

/**
 * @ingroup data_classes
 * @brief `Raster` which owns a `std::valarray`.
 * @tparam T The pixel type
 * @tparam Capacity The maximum number of pixels
 * @tparam N The raster dimension
 */
template <typename T, std::size_t Capacity, Index N = 2>
using ArrRaster = Raster<T, N, StdHolder<std::array<T, Capacity>>>;

/**
 * @ingroup data_classes
 * @brief `Raster` which owns or shares some aligned memory.
 * 
 * `AlignedBuffer<T>` is a wrapper of `T*` which may or not manage memory,
 * but at least ensures that memory is correctly aligned according to some requirements:
 * - If the `AlignedRaster` is constructed from a null pointer, then some memory will be allocated,
 *   which by default will be aligned to ensure smooth usage of SIMD instructions;
 * - If it is constructed from a non-null pointer, it will act as a `PtrRaster`, i.e. not allocate or free memory,
 *   but will verify that the memory is well aligned at construction (or throw an exception otherwise).
 */
template <typename T, Index N = 2>
using AlignedRaster = Raster<T, N, AlignedBuffer<T>>;

/**
 * @ingroup data_classes
 * @brief Data of a N-dimensional image (2D by default).
 * 
 * @tparam T The value type, which can be `const`-qualified for read-only rasters
 * @tparam N The dimension, which can be &ge; 0 for fixed dimension, or -1 for variable dimension
 * @tparam THolder The underlying data holder
 * 
 * 
 * A raster is a contiguous container for the pixel data of an image.
 * It features access and view services.
 * 
 * There are two ways of defining the dimension of a `Raster`:
 * - When the dimension is knwon at compile-time (safer),
 *   by giving the dimension parameter a positive or null value;
 * - When the dimension is known at run-time only (more flexible),
 *   by assigning `N = -1`.
 * 
 * In the former case, index and size computations are optimized, and the dimension is enforced.
 * For example, it is not possible to read a 3D image HDU as a 2D `Raster` --
 * which is nice, because an exception will be raised early!
 * In contrast, it is possible to read a 2D image HDU as a 3D `Raster` of third axis lenght =1.
 * 
 * In the latter case, the dimension may vary or be deduced from the file,
 * which is also nice sometimes but puts more responsibility on the shoulders of the user code,
 * as it should check that the actual dimension is acceptable.
 * 
 * `Raster` meets the `ContiguousContainer` requirements,
 * by extending `DataContainer` (e.g. is iterable).
 * `Raster` ensures constant-time access to elements, whatever the dimension of the data,
 * through subscipt operator `Raster::operator[]()`.
 * Bound checking and backward indexing (index <0) are enabled in `Raster::at()`.
 * 
 * `Raster` also implements some arithmetic operators by extending `ArithmeticMixin`.
 * For example, two rasters can be added, or a raster can be multiplied by a scalar.
 * Pixel-wise mathematical operations are also provided by `MathFunctionsMixin`.
 * 
 * The raster data can be viewed region-wise as a `PtrRaster`,
 * provided that the region is contiguous in memory.
 * 
 * @tspecialization{PtrRaster}
 * @tspecialization{VecRaster}
 * @tspecialization{ValRaster}
 * @tspecialization{ArrRaster}
 * @tspecialization{AlignedRaster}
 * 
 * @satisfies{ContiguousContainer}
 * @satisfies{EuclidArithmetic}
 * 
 * @see `Position` for details on the fixed- and variable-dimension cases.
 * @see \ref primer
 */
template <typename T, Index N = 2, typename THolder = DefaultHolder<T>>
class Raster : public DataContainer<T, THolder, EuclidArithmetic, Raster<T, N, THolder>> {
  friend class ImageRaster; // FIXME rm when Patch is removed

public:
  /**
   * @brief The pixel value type.
   */
  using Value = T;

  /**
   * @brief The dimension template parameter.
   * 
   * The value of `Dimension` is always `N`, irrespective of its sign.
   * In contrast, `dimension()` provides the actual, run-time dimension of the raster,
   * even in the case of a variable dimension.
   */
  static constexpr Index Dimension = N;

  /**
   * @brief The container type.
   */
  using Container = DataContainer<T, THolder, EuclidArithmetic, Raster<T, N, THolder>>;

  /// @{
  /// @group_construction

  LINX_VIRTUAL_DTOR(Raster)
  LINX_DEFAULT_COPYABLE(Raster)
  LINX_DEFAULT_MOVABLE(Raster)

  /**
   * @brief Forwarding constructor.
   * @param shape The raster shape
   * @param args The arguments to be forwarded to the data holder
   * 
   * The holder is instantiated as:
   * \code
   * Holder holder(shapeSize(shape), std::forward<TArgs>(args)...);
   * \endcode
   */
  template <typename... TArgs>
  explicit Raster(Position<N> shape = Position<N>::zero(), TArgs&&... args) :
      Container(shapeSize(shape), std::forward<TArgs>(args)...), m_shape(std::move(shape)) {}

  /**
   * @brief List-copy constructor.
   * @param shape The raster shape
   * @param list The values to be copied into the holder
   * @param args The arguments to be forwarded to the data holder
   * 
   * The holder is instantiated as:
   * \code
   * Holder holder(shapeSize(shape), std::forward<TArgs>(args)...);
   * std::copy(list.begin(), list.end(), holder.data());
   * \endcode
   */
  template <typename U, typename... TArgs>
  explicit Raster(Position<N> shape, std::initializer_list<U> list, TArgs&&... args) :
      Container(list, std::forward<TArgs>(args)...), m_shape(std::move(shape)) {
    SizeError::mayThrow(list.size(), shapeSize(shape));
  }

  /**
   * @brief Iterable-copy constructor.
   * @param shape The raster shape
   * @param iterable The values to be copied into the holder
   * @param args The arguments to be forwarded to the data holder
   * 
   * The holder is instantiated as:
   * \code
   * Holder holder(shapeSize(shape), std::forward<TArgs>(args)...);
   * std::copy(iterable.begin(), iterable.end(), holder.data());
   * \endcode
   */
  template <typename TIterable, typename std::enable_if_t<IsIterable<TIterable>::value>* = nullptr, typename... TArgs>
  explicit Raster(Position<N> shape, TIterable& iterable, TArgs&&... args) :
      Container(iterable, std::forward<TArgs>(args)...), m_shape(std::move(shape)) {
    SizeError::mayThrow(std::distance(std::begin(iterable), std::end(iterable)), shapeSize(shape));
  }

  /// @group_properties

  /**
   * @brief Get the raster shape.
   */
  const Position<N>& shape() const;

  /**
   * @brief Get the raster domain.
   * 
   * The domain is the region which spans from the first to the last pixel position.
   * 
   * \par_example
   * It can be used to loop over all pixels, e.g.:
   * \code
   * for (auto pos : raster.domain()) {
   *   processPixel(pos, raster[pos]);
   * }
   * \endcode
   */
  Box<N> domain() const;

  /**
   * @brief Check whether a given (possibly non-integral) position lies inside the raster domain.
   */
  template <typename U>
  inline bool contains(const Vector<U, N>& position) const {
    for (std::size_t i = 0; i < position.size(); ++i) { // TODO iterators
      if (position[i] < 0 || position[i] >= m_shape[i]) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Get the actual dimension.
   * 
   * This corresponds to the `N` template parameter in general,
   * or to the current dimension if variable.
   */
  Index dimension() const;

  /**
   * @brief Get the length along given axis.
   */
  template <Index I>
  Index length() const;

  /**
   * @copydoc length()
   */
  Index length(Index i) const {
    return m_shape[i];
  }

  /// @group_elements

  using Container::operator[];
  using Container::at;

  /**
   * @brief Compute the raw index of a given position.
   */
  inline Index index(const Position<N>& pos) const;

  /**
   * @brief Access the pixel value at given position.
   */
  inline const T& operator[](const Position<N>& pos) const;

  /**
   * @copybrief operator[]()
   */
  inline T& operator[](const Position<N>& pos);

  /**
   * @copybrief operator[]()
   * 
   * As opposed to `operator[]()`, negative indices are supported for backward indexing,
   * and bounds are checked.
   * @see operator[]()
   */
  inline const T& at(const Position<N>& pos) const;

  /**
   * @copybrief at()
   */
  inline T& at(const Position<N>& pos);

  /// @group_views

  /**
   * @brief Check whether a region is made of contiguous values in memory.
   * @tparam M The actual region dimension
   * 
   * A region is contiguous if and only if:
   * - For `i` < `M-1`, `front[i]` = 0 and `back[i]` = `shape[i] - 1`;
   * - For `i` > `M`, `front[i]` = `back[i]`.
   */
  template <Index M = 2>
  bool isContiguous(const Box<N>& region) const;

  /**
   * @brief Create a slice from a given region.
   * @tparam M The dimension of the slice (cannot be -1)
   * @see isContiguous()
   * @see section()
   */
  template <Index M = 2>
  const PtrRaster<const T, M> slice(const Box<N>& region) const;

  /**
   * @copybrief slice()
   */
  template <Index M = 2>
  PtrRaster<T, M> slice(const Box<N>& region);

  /**
   * @brief Create a section between given indices.
   * @param front The section front index along the last axis
   * @param back The section back index along the last axis
   * 
   * A section is a maximal slice of dimension `N` or `N`-1.
   * For example, a 3D section of a 3D raster of shape (x, y, z)
   * is a 3D raster of shape (x, y, t) where `t` < `z`,
   * while a 2D section of it is a 2D raster of shape (x, y).
   * 
   * If needed, `section()` can be applied recursively,
   * e.g. to get the x-line at `z` = 4 and `y` = 2:
   * \code
   * auto line = raster.section(4).section(2);
   * \endcode
   * 
   * @see slice()
   */
  const PtrRaster<const T, N> section(Index front, Index back) const;

  /**
   * @copybrief section(Index,Index)const
   */
  PtrRaster<T, N> section(Index front, Index back);

  /**
   * @brief Create a section at given.
   */
  const PtrRaster<const T, N == -1 ? -1 : N - 1> section(Index index) const;

  /**
   * @copybrief section(Index)const
   */
  PtrRaster<T, N == -1 ? -1 : N - 1> section(Index index);

  /**
   * @brief Create a patch from given region.
   * 
   * A patch is a view of the raster data contained in a region.
   * As opposed to a slice or a section, a patch is not necessarily contiguous in memory.
   * 
   * Patches are iterable.
   * 
   * @see isContiguous()
   * @see slice()
   * @see section()
   */
  template <typename TRegion>
  const Patch<const T, const Raster<T, N, THolder>, TRegion> patch(TRegion region) const;

  /**
   * @copybrief patch().
   */
  template <typename TRegion>
  Patch<T, Raster<T, N, THolder>, TRegion> patch(TRegion region);

  /// @}

private:
  /**
   * @brief Raster shape, i.e. length along each axis.
   */
  Position<N> m_shape;
};

/**
 * @relates Raster
 * @brief Equality operator.
 */
template <typename T, Index N, typename THolder, typename U, Index M, typename UHolder>
bool operator==(const Raster<T, N, THolder>& lhs, const Raster<U, M, UHolder>& rhs) {
  if (lhs.shape() != rhs.shape()) {
    return false;
  }
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
/**
 * @relates Raster
 * @brief Inequality operator.
 */
template <typename T, Index N, typename THolder, typename U, Index M, typename UHolder>
bool operator!=(const Raster<T, N, THolder>& lhs, const Raster<U, M, UHolder>& rhs) {
  return not(lhs == rhs);
}

/**
 * @relates Raster
 * @brief Shortcut to create a raster from a shape and data without specifying the template parameters.
 * @tparam T The pixel type, should not be specified (automatically deduced)
 * @tparam Longs The axes lengths, should not be specified (automatically deduced)
 * @param data The raster values, which can be either a pointer (or C array) or a vector
 * @param shape The shape as a comma-separated list of `Index`s
 * 
 * \par_example
 * \code
 * Given:
 * - Index width, height, depth: The axes lengths;
 * - float* ptr: The pixel values as a pointer;
 * - std::vector<float> vec: The pixel values as a vector;
 * 
 * auto ptrRaster2D = rasterize(ptr, width, height);
 * auto ptrRaster3D = rasterize(ptr, width, height, depth);
 * auto vecRaster2D = rasterize(vec, width, height); // The vector is copied
 * auto vecRaster3D = rasterize(std::move(vec), width, height, depth); // The vector is moved
 * \endcode
 */
template <typename TContainer, typename... Longs>
Raster<typename TContainer::value_type, sizeof...(Longs), StdHolder<TContainer>>
rasterize(TContainer&& data, Longs... shape) {
  return Raster<typename TContainer::value_type, sizeof...(Longs), StdHolder<TContainer>> {
      {shape...},
      std::forward<TContainer>(data)};
}

/**
 * @relates Raster
 * @copybrief rasterize()
 */
template <typename T, typename... Longs>
PtrRaster<T, sizeof...(Longs)> rasterize(T* data, Longs... shape) {
  return PtrRaster<T, sizeof...(Longs)> {{shape...}, data};
}

/**
 * @relates Raster
 * @brief Identity.
 * 
 * This function is provided for compatibility with `Extrapolator`
 * in cases where functions accept either a `Raster` or an `Extrapolator`.
 */
template <typename T, Index N, typename THolder>
const Raster<T, N, THolder>& rasterize(const Raster<T, N, THolder>& in) {
  return in;
}

#define LINX_MATH_COMPLEX_TO_REAL(function) \
  template <typename T, Index N, typename THolder> \
  Raster<T, N> function(const Raster<std::complex<T>, N, THolder>& in) { \
    Raster<T, N> out(in.shape()); \
    std::transform(in.begin(), in.end(), out.begin(), [](const auto& e) { \
      return std::function(e); \
    }); \
    return out; \
  }
// FIXME same THolder?

/// @ingroup pixelwise
/// @{

LINX_MATH_COMPLEX_TO_REAL(real) ///< Apply `std::real()` @ingroup pixelwise
LINX_MATH_COMPLEX_TO_REAL(imag) ///< Apply `std::imag()` @ingroup pixelwise
LINX_MATH_COMPLEX_TO_REAL(abs) ///< Apply `std::abs()` @ingroup pixelwise
LINX_MATH_COMPLEX_TO_REAL(arg) ///< Apply `std::arg()` @ingroup pixelwise
LINX_MATH_COMPLEX_TO_REAL(norm) ///< Apply `std::norm()` @ingroup pixelwise
// FIXME conj, polar

/// @}

#undef LINX_MATH_COMPLEX_TO_REAL

/**
 * @relates Raster
 * @brief Generate a random raster.
 * 
 * Pixel values are uniformly distributed between the type's half min and half max.
 */
template <typename T, Index N = 2>
Raster<T, N> random(const Position<N>& shape) {
  Raster<T, N> out(shape);
  out.generate(UniformNoise<T>());
  return out;
}

} // namespace Linx

#include "LinxCore/Patch.h"
#include "LinxCore/impl/Raster.hpp"

#endif
