// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXDATA_RASTER_H
#define _LINXDATA_RASTER_H

#include "Linx/Base/AlignedBuffer.h"
#include "Linx/Base/Exceptions.h"
#include "Linx/Base/Random.h"
#include "Linx/Base/mixins/DataContainer.h"
#include "Linx/Data/Box.h"
#include "Linx/Data/Line.h"
#include "Linx/Data/Patch.h"
#include "Linx/Data/Sequence.h"

#include <complex>
#include <cstdint>
#include <string>
#include <valarray>
#include <vector>

/**
 * @brief Extensible ND image laboratory.
 * 
 * An ND image processing library developed and maintained by the French space agency,
 * Centre National d'Etudes Spatiales.
 * @see https://github.com/cnes
 * @see https://cnes.fr/en
 */
namespace Linx {

/// @cond
// Issue with forward declarations: https://github.com/doxygen/doxygen/issues/8177

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
 * @brief `Raster` which owns a `std::array`.
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
 * Non contiguous view are implemented as `Patch`es, which can have arbitrarily shaped regions.
 * 
 * @tspecialization{PtrRaster}
 * @tspecialization{VecRaster}
 * @tspecialization{ValRaster}
 * @tspecialization{ArrRaster}
 * @tspecialization{AlignedRaster}
 * 
 * @satisfies{ContiguousContainer}
 * 
 * @satisfies{VectorArithmetic}
 * 
 * @satisfies{EuclidArithmetic}
 * 
 * @see `Position` for details on the fixed- and variable-dimension cases.
 * @see \ref primer
 */
template <typename T, Index N = 2, typename THolder = DefaultHolder<T>>
class Raster : public Dimensional<N>, public DataContainer<T, THolder, EuclidArithmetic, Raster<T, N, THolder>> {
public:

  /**
   * @brief The pixel value type.
   */
  using Value = T;

  /**
   * @brief The container type.
   */
  using Container = DataContainer<T, THolder, EuclidArithmetic, Raster<T, N, THolder>>;

  /**
   * @brief The section type.
   * 
   * A section is a contiguous view of dimension N-1, like a 2D plane in a 3D raster or a row in a 2D raster.
   * 
   * In contrast, a chunk has dimension N.
   * For example, a 3D chunk of a 3D raster of shape (x, y, z)
   * is a 3D patch of shape (x, y, t) where `t` < `z`,
   * while a 2D section of it is a 2D raster of shape (x, y).
   * 
   * If needed, `section()` can be applied recursively,
   * e.g. to get the x-line at `z` = 4 and `y` = 2:
   * \code
   * auto line = raster.section(4).section(2);
   * \endcode
   * 
   * If the index along the last dimension must be known from the section, then a chunk of thickness 1 should be used instead.
   * 
   * \code
   * auto raster3d = Raster<int, 3>({3, 4, 5}).random();
   * auto chunk3d = raster3d.chunk(3);
   * auto section2d = raster3d.section(3);
   * auto z = chunk3d.domain().front()[2]; // Unavailable to section2d
   * \endcode
   */
  using Section = PtrRaster<T, Raster::OneLessDimension>;

  /**
   * @brief The constant section type.
   * @see Section
   */
  using ConstSection = PtrRaster<const T, Raster::OneLessDimension>;

  /**
   * @brief The chunk type.
   * 
   * A chunk is a contiguous view of dimension N, like a consecutive set of rows in a 2D raster.
   */
  using Chunk = Patch<T, Raster, Box<N>, true>;

  /**
   * @brief The read-only chunk type.
   * @see Chunk
   */
  using ConstChunk = Patch<const T, const Raster, Box<N>, true>;

  /**
   * @brief The row type.
   * 
   * A row is a contiguous view of dimension 1, along axis 0.
   */
  using Row = Patch<T, Raster, Line<0, N>, true>;

  /**
   * @brief The read-only row type.
   * @see Row
   */
  using ConstRow = Patch<const T, const Raster, Line<0, N>, true>;

  /**
   * @brief The tile type.
   * 
   * A tile of dimension M is generally a non-contiguous view whose domain is a box of dimension M.
   */
  template <Index M>
  using Tile = Patch<T, Raster, Box<M>>;

  /**
   * @brief The read-only tile type.
   * @see Tile
   */
  template <Index M>
  using ConstTile = Patch<const T, const Raster, Box<M>>;

  /**
   * @brief The profile type.
   * 
   * A profile along axis I is a generally non-contiguous view of dimension 1.
   * It is contiguous if I = 0.
   */
  template <Index I>
  using Profile = Patch<T, Raster<T, N, THolder>, Line<I, N>, I == 0>;

  /**
   * @brief The read-only profile type.
   * @see Profile
   */
  template <Index I>
  using ConstProfile = Patch<const T, const Raster, Line<I, N>, I == 0>;

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
   * Holder holder(shape_size(shape), std::forward<TArgs>(args)...);
   * \endcode
   */
  template <typename... TArgs>
  explicit Raster(Position<N> shape = Position<N>::zero(), TArgs&&... args) :
      Container(shape_size(shape), std::forward<TArgs>(args)...), m_shape(std::move(shape))
  {}

  /**
   * @brief List-copy constructor.
   * @param shape The raster shape
   * @param list The values to be copied into the holder
   * @param args The arguments to be forwarded to the data holder
   * 
   * The holder is instantiated as:
   * \code
   * Holder holder(shape_size(shape), std::forward<TArgs>(args)...);
   * std::copy(list.begin(), list.end(), holder.data());
   * \endcode
   */
  template <typename... TArgs>
  explicit Raster(Position<N> shape, std::initializer_list<T> list, TArgs&&... args) :
      Container(list.begin(), list.end(), std::forward<TArgs>(args)...), m_shape(std::move(shape))
  {
    SizeError::may_throw(this->size(), shape_size(shape));
  }

  /**
   * @brief Range-copy constructor.
   * @param shape The raster shape
   * @param range The values to be copied into the holder
   * @param args The arguments to be forwarded to the data holder
   * 
   * The holder is instantiated as:
   * \code
   * Holder holder(shape_size(shape), std::forward<TArgs>(args)...);
   * std::copy(range.begin(), range.end(), holder.data());
   * \endcode
   */
  template <typename TRange, typename std::enable_if_t<IsRange<TRange>::value>* = nullptr, typename... TArgs>
  explicit Raster(Position<N> shape, TRange& range, TArgs&&... args) :
      Container(range.begin(), range.end(), std::forward<TArgs>(args)...), m_shape(std::move(shape))
  {
    SizeError::may_throw(this->size(), shape_size(shape));
  }

  /**
   * @brief Patch-copy constructor.
   * @param patch The box- or grid-based patch to be copied (can be an extrapolator).
   */
  template <typename U, typename TRaster, typename TRegion>
  explicit Raster(const Patch<U, TRaster, TRegion>& patch) : Raster(patch.domain().shape(), patch)
  {}

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
   *   process_pixel(pos, raster[pos]);
   * }
   * \endcode
   */
  Box<N> domain() const;

  /**
   * @brief Check whether a given (possibly non-integral) position lies inside the raster domain.
   */
  template <typename U>
  inline bool contains(const Vector<U, N>& position) const
  {
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
  Index length(Index i) const
  {
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
  bool is_contiguous(const Box<N>& region) const;

  /**
   * @brief Create a slice from a given region.
   * @tparam M The dimension of the slice (cannot be -1)
   * @see is_contiguous()
   * @see section()
   */
  template <Index M = 2>
  PtrRaster<const T, M> slice(const Box<N>& region) const;

  /**
   * @copybrief slice()
   */
  template <Index M = 2>
  PtrRaster<T, M> slice(const Box<N>& region);

  /**
   * @brief Create a chunk between given indices.
   * @param front The chunk front index along the last axis
   * @param back The chunk back index along the last axis
   * @see Chunk
   */
  ConstChunk chunk(Index front, Index back) const;

  /**
   * @copybrief chunk(Index,Index)const
   */
  Chunk chunk(Index front, Index back);

  /**
   * @brief Create a chunk at given index.
   * @see Chunk
   */
  ConstChunk chunk(Index index) const
  {
    return chunk(index, index);
  }

  /**
   * @copybrief chunk(Index)const
   */
  Chunk chunk(Index index)
  {
    return chunk(index, index);
  }

  /**
   * @brief Create a section at given index.
   */
  ConstSection section(Index index) const;

  /**
   * @copybrief section(Index)const
   */
  Section section(Index index);

  /**
   * @brief Create a row at given position.
   * @param i The row index following the data ordering
   * @param position The row position with dimension N - 1 (the coordinate along the first axis is not specified)
   */
  ConstRow row(Index i) const
  {
    return row(Position<1> {i}); // FIXME accept other dimensions
  }

  /**
   * @copybrief row(Index)const
   */
  Row row(Index i)
  {
    return row(Position<1> {i}); // FIXME accept other dimensions
  }

  /**
   * @copybrief row(Index)const
   */
  ConstRow row(const Position<Raster::OneLessDimension>& position) const;

  /**
   * @copybrief row(Index)const
   */
  Row row(const Position<Raster::OneLessDimension>& position);

  /**
   * @brief Create a line-patch at given position.
   */
  template <Index I>
  ConstProfile<I> profile(const Position<Raster::OneLessDimension>& position) const;

  /**
   * @brief Create a line-patch at given position.
   */
  template <Index I>
  Profile<I> profile(const Position<Raster::OneLessDimension>& position);

  /**
   * @brief Create a patch from given region.
   * 
   * A patch is a view of the raster data contained in a region.
   * As opposed to a slice or a section, a patch is not necessarily contiguous in memory.
   * 
   * Patches are iterable.
   * 
   * @see is_contiguous()
   * @see slice()
   * @see section()
   */
  template <typename TRegion, typename std::enable_if_t<is_region<TRegion>()>* = nullptr>
  Patch<const T, const Raster, std::decay_t<TRegion>> operator()(TRegion&& region) const
  {
    return Patch<const T, const Raster, std::decay_t<TRegion>>(*this, LINX_FORWARD(region));
  }

  /**
   * @copybrief operator()(TRegion)const
   */
  template <typename TRegion, typename std::enable_if_t<is_region<TRegion>()>* = nullptr>
  Patch<T, Raster, std::decay_t<TRegion>> operator()(TRegion&& region)
  {
    return Patch<T, Raster, std::decay_t<TRegion>>(*this, LINX_FORWARD(region));
  }

  /**
   * @copybrief operator()(TRegion)const
   */
  auto operator()(Position<N> p0) const
  {
    return Patch<const T, const Raster, Position<N>>(*this, LINX_MOVE(p0)); // FIXME as PtrRaster<T, 1>
  }

  /**
   * @copybrief operator()(TRegion)const
   */
  auto operator()(Position<N> p0)
  {
    return Patch<T, Raster, Position<N>>(*this, LINX_MOVE(p0)); // FIXME as PtrRaster<T, 1>
  }

  /**
   * @copybrief operator()(TRegion)const
   */
  template <typename... TPositions>
  auto operator()(Position<N> p0, TPositions&&... ps) const
  {
    using R = ArrSequence<Position<N>, sizeof...(TPositions) + 1>;
    return Patch<const T, const Raster, R>(*this, {LINX_MOVE(p0), LINX_FORWARD(ps)...});
  }

  /**
   * @copybrief operator()(TRegion)const
   */
  template <typename... TPositions>
  auto operator()(Position<N> p0, TPositions&&... ps)
  {
    using R = ArrSequence<Position<N>, sizeof...(TPositions) + 1>;
    return Patch<T, Raster, R>(*this, {LINX_MOVE(p0), LINX_FORWARD(ps)...});
  }

  /// @}

private:

  /**
   * @brief Raster shape, i.e. length along each axis.
   */
  Position<N> m_shape;
};

/// @cond
namespace Internal {

template <typename>
struct IsRasterImpl : std::false_type {};

template <typename T, Index N, typename THolder>
struct IsRasterImpl<Raster<T, N, THolder>> : std::true_type {};

} // namespace Internal
/// @endcond

/**
 * @relatesalso Raster
 * @brief Check whether a class can be used as a raster.
 */
template <typename T>
constexpr bool is_raster()
{
  return Internal::IsRasterImpl<std::decay_t<T>>::value;
}

/**
 * @relatesalso Raster
 * @brief Equality operator.
 */
template <typename T, Index N, typename THolder, typename U, Index M, typename UHolder>
bool operator==(const Raster<T, N, THolder>& lhs, const Raster<U, M, UHolder>& rhs)
{
  if (lhs.shape() != rhs.shape()) {
    return false;
  }
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
/**
 * @relatesalso Raster
 * @brief Inequality operator.
 */
template <typename T, Index N, typename THolder, typename U, Index M, typename UHolder>
bool operator!=(const Raster<T, N, THolder>& lhs, const Raster<U, M, UHolder>& rhs)
{
  return not(lhs == rhs);
}

/**
 * @relatesalso Raster
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
 * auto ptrraster2d = rasterize(ptr, width, height);
 * auto ptrraster3d = rasterize(ptr, width, height, depth);
 * auto vecraster2d = rasterize(vec, width, height); // The vector is copied
 * auto vecraster3d = rasterize(std::move(vec), width, height, depth); // The vector is moved
 * \endcode
 */
template <typename TContainer, typename... Longs>
Raster<typename TContainer::value_type, sizeof...(Longs), StdHolder<TContainer>>
rasterize(TContainer&& data, Longs... shape)
{
  return Raster<typename TContainer::value_type, sizeof...(Longs), StdHolder<TContainer>> {
      {shape...},
      std::forward<TContainer>(data)};
}

/**
 * @relatesalso Raster
 * @copybrief rasterize()
 */
template <typename T, typename... Longs>
PtrRaster<T, sizeof...(Longs)> rasterize(T* data, Longs... shape)
{
  return PtrRaster<T, sizeof...(Longs)> {{shape...}, data};
}

/**
 * @relatesalso Raster
 * @brief Identity.
 * 
 * This function is provided for compatibility with `Extrapolation`
 * in cases where functions accept either a `Raster` or an `Extrapolation`.
 */
template <typename T, Index N, typename THolder>
const Raster<T, N, THolder>& rasterize(const Raster<T, N, THolder>& in)
{
  return in;
}

#define LINX_MATH_COMPLEX_TO_REAL(function) \
  template <typename T, Index N, typename THolder> \
  Raster<T, N> function(const Raster<std::complex<T>, N, THolder>& in) \
  { \
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
 * @relatesalso Raster
 * @brief Generate a random raster.
 * 
 * Pixel values are uniformly distributed between the type's half min and half max.
 */
template <typename T, Index N = 2>
Raster<T, N> random(const Position<N>& shape)
{
  Raster<T, N> out(shape);
  out.generate(UniformNoise<T>());
  return out;
}

} // namespace Linx

#include "Linx/Data/Patch.h"
#include "Linx/Data/impl/Raster.hpp"

#endif
