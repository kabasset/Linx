// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_IMAGE_H
#define LINX_DATA_IMAGE_H

#include "Linx/Base/Containers.h"
#include "Linx/Base/Functional.h"
#include "Linx/Base/Slice.h"
#include "Linx/Base/Types.h"
#include "Linx/Base/mixins/Data.h"
#include "Linx/Data/Box.h"
#include "Linx/Data/Sequence.h"

#include <Kokkos_Core.hpp>
#include <Kokkos_StdAlgorithms.hpp>
#include <concepts>
#include <string>

namespace Linx {

/**
 * @ingroup arrays
 * @brief Non-resizable ND array.
 * 
 * @tparam T The element value type
 * @tparam N The rank, or -1 for dynamic rank
 * @tparam TContainer The underlying container type
 * 
 * By default, image elements are default-initialized.
 * Copy constructor and copy assignment operator perform shallow copy.
 * 
 * @see arrays
 * @see `DataMixin`
 * @see `RangeMixin`
 */
template <typename T, int N, typename TContainer = ImageContainer<T, N>>
class Image :
    public DataMixin<T, DataArithmeticMixin<T, Image<T, N, TContainer>>, Image<T, N, TContainer>>,
    public RangeMixin<is_contiguous<TContainer>(), T, Image<T, N, TContainer>> {
private:

  static constexpr int kokkos_max_dyn_rank = (N == -1 ? 7 : N); ///< The max dynamic rank supported by Kokkos
  // TODO make public variable, as well as kokkos_max_rank = 8 and kokkos_max_op_rank = 6?

public:

  static constexpr int n = N; ///< The rank parameter
  using Container = TContainer; ///< The underlying container type
  using Shape = Position<N>; ///< The shape type
  using Domain = Box<N>; ///< The domain type

  using memory_space = typename Container::memory_space;
  using execution_space = typename Container::execution_space;

  using value_type = typename Container::value_type; ///< The possibly const-qualified element type
  using element_type = std::remove_cvref_t<value_type>; ///< The element type
  using size_type = typename Container::size_type; ///< The index and size type
  using difference_type = std::ptrdiff_t; ///< The index difference type
  using reference = typename Container::reference_type; ///< The element reference type
  using pointer = typename Container::pointer_type; ///< The element pointer type

  /**
   * @brief Constructor.
   * 
   * @param shape The image extents along each axis
   * 
   * @warning If the rank is static (`n != -1`), the extent count must match it.
   */
  explicit Image(std::integral auto... shape) : Image("", shape...) {}

  /**
   * @brief Constructor.
   * 
   * @param label The image label
   * @param shape The image extent along each axis
   * 
   * @warning If the rank is static (`n != -1`), the extent count must match it.
   */
  explicit Image(const std::string& label, std::integral auto... shape) : m_container(label, shape...) {}

  /**
   * @brief Constructor.
   * 
   * @param shape The image extents along each axis
   * 
   * @warning If the rank is static (`n != -1`), the shape rank must match it.
   */
  template <std::integral TInt, typename UContainer>
  explicit Image(const Sequence<TInt, n, UContainer>& shape) : Image("", shape) // TODO use LegacyArray?
  {}

  /**
   * @brief Constructor.
   * 
   * @param label The image label
   * @param shape The image extents along each axis
   * 
   * @warning If the rank is static (`n != -1`), the shape rank must match it.
   */
  template <std::integral TInt, typename UContainer>
  explicit Image(const std::string& label, const Sequence<TInt, n, UContainer>& shape) :
      Image(label, shape, std::make_index_sequence<kokkos_max_dyn_rank>()) // TODO use LegacyArray?
  {}

  /**
   * @copydoc Image()
   */
  [[deprecated]] KOKKOS_INLINE_FUNCTION explicit Image(const Container& container) : m_container(container) {}

  /**
   * @copydoc Image()
   */
  [[deprecated]] KOKKOS_INLINE_FUNCTION explicit Image(Container&& container) : m_container(LINX_FORWARD(container)) {}

  /**
   * @brief Forwarding constructor.
   * @param args The parameters forwarded to the container's constructor
   */
  template <typename... TArgs>
  KOKKOS_INLINE_FUNCTION explicit Image(Forward, TArgs&&... args) : m_container(LINX_FORWARD(args)...)
  {}

  /**
   * @brief Wrapping constructor.
   * @param data The wrapped data
   * @param shape The image extents along each axis
   * 
   * The resulting image does not own the data.
   * It won't manage its memory or ensure it is valid.
   * 
   * @warning If the rank is static (`n != -1`), the extent count must match it.
   */
  template <typename U>
  explicit Image(Wrap<U*> data, std::integral auto... shape) : m_container(data.value, shape...)
  {}

  /**
   * @brief Wrapping constructor.
   * @param data The wrapped data
   * @param shape The image extents along each axis
   * 
   * The resulting image does not own the data.
   * It won't manage its memory or ensure it is valid.
   * 
   * @warning If the rank is static (`n != -1`), the shape rank must match it.
   */
  template <typename U, std::integral TInt, typename UContainer>
  explicit Image(Wrap<U*> data, const Sequence<TInt, n, UContainer>& shape) :
      Image(data, shape, std::make_index_sequence<kokkos_max_dyn_rank>()) // TODO use LegacyArray?
  {}

  /**
   * @brief Image rank.
   */
  KOKKOS_INLINE_FUNCTION int rank() const
  {
    if constexpr (n == -1) {
      return Kokkos::rank(m_container);
    } else {
      return n;
    }
  }

  /**
   * @brief Image extent along a given axis.
   */
  KOKKOS_INLINE_FUNCTION int extent(std::integral auto i) const
  {
    return m_container.extent_int(i);
  }

  /**
   * @brief Image extents along all axes. 
   */
  Shape shape() const
  {
    Shape out(rank());
    for (int i = 0; i < rank(); ++i) {
      out[i] = m_container.extent_int(i);
    }
    return out;
  }

  /**
   * @brief Image domain. 
   */
  Domain domain() const
  {
    return domain(m_container);
  }

  KOKKOS_INLINE_FUNCTION auto stride(std::integral auto i) const
  {
    return m_container.stride(i);
  }

  /**
   * @brief Memory striding.
   */
  Position<n> strides() const
  {
    std::vector<Index> longer(rank() + 1);
    m_container.stride(longer.data());
    return Position<n>("strides", longer.data(), longer.data() + rank());
  }

  /**
   * @brief Underlying container.
   */
  KOKKOS_INLINE_FUNCTION const Container& container() const
  {
    return m_container;
  }

  /**
   * @brief Access the first element.
   * 
   * As opposed to `data()`, which is the pointer to the allocated memory,
   * `&front()` is a pointer to the first element.
   * Therefore, `data()` can be less than `&front()`, e.g. for alignment purposes.
   */
  KOKKOS_INLINE_FUNCTION reference front() const
  {
    return origin();
  }

  /**
   * @brief Access the element at position 0.
   */
  KOKKOS_INLINE_FUNCTION reference origin() const
  {
    return m_container.access(0, 0, 0, 0, 0, 0, 0, 0);
    // FIXME not scalable if max rank goes >8 some day
  }

  /**
   * @brief Access the last element.
   */
  KOKKOS_INLINE_FUNCTION reference back() const
  {
    Kokkos::Array<int, 8> p;
    for (int i = 0; i < rank(); ++i) {
      p[i] = extent(i) - 1;
    }
    return m_container.access(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
    // FIXME not scalable if max rank goes >8 some day
  }

  /**
   * @brief Access the element at given position.
   */
  KOKKOS_INLINE_FUNCTION reference operator()(std::integral auto... position) const
  {
    return m_container(position...);
  }

  /**
   * @brief Access the element at given position.
   */
  template <std::integral TInt = int, int M = n>
  KOKKOS_INLINE_FUNCTION reference operator[](const GPosition<TInt, M>& position) const // FIXME use LegacyArray?
  {
    // FIXME validate M
    return at(position, std::make_index_sequence<kokkos_max_dyn_rank>());
  }

  /**
   * @brief Slice the image as a shallow copy.
   * @param region The slicing as a `Box`
   */
  template <typename U, int M>
  auto operator[](const GBox<U, M>& region) const // not __device__ because of `region & domain()`
  {
    const auto& crop = region & domain();
    using Container = decltype(slice_all(crop, std::make_index_sequence<M>()));
    return Image<T, Container::rank(), Container>(Forward {}, slice_all(crop, std::make_index_sequence<M>()));
  }

  /**
   * @brief Slice the image as a shallow copy.
   * @param region The slicing region as a `Slice`
   * 
   * The `Slice` must have either a rank of:
   * - 1, in which case the slicing is performed on the last axis only;
   * - `n`, in which case the slicing is performed on all axes.
   * 
   * As opposed to patches:
   * - If the slice contains singletons, the associated axes are droped;
   * - The domain of the resulting image starts at position 0;
   * - The parent image can safely be destroyed.
   * 
   * @see `Patch`
   */
  template <typename U, typename... TFuncs>
  auto operator[](const Slice<U, TFuncs...>& region) const // not __device__ because of `region & domain()`
  {
    const auto& crop = region & domain(); // Resolve Kokkos::ALL to drop offsets with subview
    if constexpr (sizeof...(TFuncs) == 1) {
      using Container = decltype(slice_last(std::make_index_sequence<n - 1>(), crop));
      return Image<T, Container::rank(), Container>(Forward {}, slice_last(std::make_index_sequence<n - 1>(), crop));
    } else {
      // FIXME assert sizeoff...(TFuncs) == n?
      using Container = decltype(slice_all(crop, std::make_index_sequence<sizeof...(TFuncs)>()));
      return Image<T, Container::rank(), Container>(
          Forward {},
          slice_all(crop, std::make_index_sequence<sizeof...(TFuncs)>()));
    }
  }

private:

  /**
   * @brief Helper constructor to unroll shape.
   */
  template <typename TShape, std::size_t... Is>
  Image(const std::string& label, const TShape& shape, std::index_sequence<Is...>) :
      Image(label, get_or<Is>(shape, KOKKOS_INVALID_INDEX)...)
  {}

  /**
   * @brief Helper constructor to unroll shape.
   */
  template <typename U, typename TShape, std::size_t... Is>
  Image(Wrap<U*> data, const TShape& shape, std::index_sequence<Is...>) :
      Image(data, get_or<Is>(shape, KOKKOS_INVALID_INDEX)...)
  {}

  /**
   * @brief Helper accessor to unroll position.
   */
  template <typename TPosition, std::size_t... Is>
  KOKKOS_INLINE_FUNCTION reference at(const TPosition& position, std::index_sequence<Is...>) const // FIXME at_impl?
  {
    return operator()(get_or<Is>(position, 0)...); // FIXME at()?
  }

  /**
   * @brief Helper function for 0-based fixed-rank containers.
   */
  template <typename... TArgs>
  static Domain domain(const Kokkos::View<TArgs...>& container)
  {
    Shape start("Image domain start");
    Shape stop("Image domain stop");
    for (int i = 0; i < n; ++i) {
      start[i] = 0;
      stop[i] = container.extent_int(i);
    }
    return {LINX_MOVE(start), LINX_MOVE(stop)};
  }

  /**
   * @brief Helper function for 0-based dynamic rank containers.
   */
  template <typename... TArgs>
  static Domain domain(const Kokkos::DynRankView<TArgs...>& container)
  {
    auto rank = container.rank();
    Shape start("Image domain start", rank);
    Shape stop("Image domain stop", rank);
    for (decltype(rank) i = 0; i < rank; ++i) {
      start[i] = 0;
      stop[i] = container.extent_int(i);
    }
    return {LINX_MOVE(start), LINX_MOVE(stop)};
  }

  /**
   * @brief Slice along each axis.
   */
  template <typename TSlice, std::size_t... Is>
  auto slice_all(const TSlice& slice, std::index_sequence<Is...>) const
  {
    return Kokkos::subview(m_container, kokkos_slice(get<Is>(slice))...);
  }

  /**
   * @brief Slice along the last axis.
   */
  template <typename TSlice, std::size_t... Is>
  auto slice_last(std::index_sequence<Is...>, const TSlice& slice) const
  {
    using Prepend = std::array<Kokkos::ALL_t, sizeof...(Is)>;
    return Kokkos::subview(m_container, (typename std::tuple_element<Is, Prepend>::type {})..., kokkos_slice(slice));
  }

private:

  /**
   * @brief The underlying container.
   */
  Container m_container;
};

template <typename T, std::integral... TExtents>
Image(Wrap<T*>, TExtents...) -> Image<T, sizeof...(TExtents)>;

template <typename T, typename U, int N, typename TContainer>
Image(Wrap<T*>, Sequence<U, N, TContainer>) -> Image<T, N>;

template <typename T, typename U, int N>
Image(Wrap<T*>, U (&&)[N]) -> Image<T, N>;

template <typename T>
struct IsImage : std::false_type {};

template <typename T, int N, typename... TArgs>
struct IsImage<Image<T, N, TArgs...>> : std::true_type {};

template <typename T>
concept AnyImage = IsImage<T>::value; // is_specialization won't work with non-type template parameters

/**
 * @brief Perform a shallow copy of an image, as a readonly image.
 * 
 * If the input image is aleady readonly, then this is a no-op.
 */
template <typename T, int N, typename TContainer>
KOKKOS_INLINE_FUNCTION decltype(auto) as_readonly(const Image<T, N, TContainer>& in)
{
  if constexpr (std::is_const_v<T>) {
    return in;
  } else {
    using Out = Image<const T, N, typename Rebind<TContainer>::AsReadonly>;
    return Out(Forward {}, in.container());
  }
}

/**
 * @brief Perform a shallow copy of an image, as an atomic image.
 */
template <typename T, int N, typename TContainer>
KOKKOS_INLINE_FUNCTION decltype(auto) as_atomic(const Image<T, N, TContainer>& in)
{
  using Out = Image<T, N, typename Rebind<TContainer>::AsAtomic>;
  return Out(Forward {}, in.container());
}

/**
 * @brief Copy the data to host if on device.
 */
template <typename T, int N, typename TContainer>
decltype(auto) on_host(const Image<T, N, TContainer>& in)
{
  return on_device<Kokkos::HostSpace>(in);
}

/**
 * @brief Copy the data to a given memory space if not already accessible from it.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace::memory_space, typename T, int N, typename TContainer>
decltype(auto) on_device(const Image<T, N, TContainer>& in)
{
  if constexpr (Kokkos::SpaceAccessibility<TSpace, typename TContainer::memory_space>::accessible) {
    return in;
  } else {
    auto container = Kokkos::create_mirror_view_and_copy(TSpace(), in.container());
    return Image<T, N, decltype(container)>(Forward {}, LINX_MOVE(container));
  }
}

/**
 * @brief Create an image with the same memory layout as another image.
 * @tparam U The type of the elements in the new image (defaults to the type of the elements in the input image)
 */
template <typename U = void, typename T, int N, typename TContainer>
auto same_layout(const std::string& label, const Image<T, N, TContainer>& in)
{
  return Image<typename Rebind<T>::As<U>, N, typename Rebind<TContainer>::As<U>>(
      Forward(),
      same_layout<U>(label, in.container()));
}

/**
 * @brief Contiguous image on host with row-major ordering.
 * 
 * This specialization is mostly provided for interfacing with legacy code.
 * Row-major ordering means that the elements are contiguous along the first index,
 * which is conventionally considered to be the index along a row:
 * 
 * \code
 * Raster<int, 2> raster(shape);
 * assert(&raster(x, y) + 1 == &raster(x + 1, y));
 * \endcode
 * 
 * Said otherwise, the stride along axis 0 is 1.
 */
template <typename T, int N = 2>
using Raster = Image<T, N, ImageContainer<T, N, Kokkos::LayoutLeft, Kokkos::HostSpace>>;

/**
 * @ingroup creation
 * @brief Create a 1D image made of a single row.
 */
template <typename T, int N>
auto rowwise(const std::string& label, T (&&row)[N])
{
  auto raster = Raster<T, 1>(Wrap(row), N);
  auto out = Image<T, 1>(label, N);
  Kokkos::deep_copy(out.container(), raster.container());
  return out;
}

/**
 * @ingroup creation
 * @brief Create a 2D image from a collection of rows.
 */
template <typename T, int N0, int N1>
auto rowwise(const std::string& label, T (&&rows)[N1][N0])
{
  T* data = *rows;
  auto raster = Raster<T, 2>(Wrap(data), N0, N1);
  auto out = Image<T, 2>(label, N0, N1);
  Kokkos::deep_copy(out.container(), raster.container());
  return out;
}

/**
 * @ingroup creation
 * @brief Create a 3D image from a collection of rows.
 */
template <typename T, int N0, int N1, int N2>
auto rowwise(const std::string& label, T (&&rows)[N2][N1][N0])
{
  T* data = **rows;
  auto raster = Raster<T, 3>(Wrap(data), N0, N1, N2);
  auto out = Image<T, 3>(label, N0, N1, N2);
  Kokkos::deep_copy(out.container(), raster.container());
  return out;
}

/**
 * @ingroup creation
 * @brief Image filled with a single value.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T, std::integral... Is>
auto fill(const std::string& label, const T& value, Is... shape)
{
  static constexpr auto n = sizeof...(Is);
  return Image<T, n, ImageContainer<T, n, TSpace>>(label, shape...).fill(value); // TODO uninitialized
}

/**
 * @ingroup creation
 * @brief Generate an image.
 * @param label The label
 * @param func The generator
 * @param shape The shape
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
auto generate(const std::string& label, const auto& func, std::integral auto... shape)
{
  using T = std::remove_cvref_t<decltype(func(shape...))>;
  static constexpr auto n = sizeof...(shape);
  return Image<T, n, ImageContainer<T, n, TSpace>>(label, shape...).copy_from(func);
}

/**
 * @brief Iterator to the beginning of a contiguous image.
 */
template <typename T, int N, typename TContainer>
  requires(is_contiguous<TContainer>())
auto begin(const Image<T, N, TContainer>& image)
{
  return image.data();
}

/**
 * @brief Iterator to the end of a contiguous image.
 */
template <typename T, int N, typename TContainer>
  requires(is_contiguous<TContainer>())
auto end(const Image<T, N, TContainer>& image)
{
  return begin(image) + image.size();
}

} // namespace Linx

#endif
