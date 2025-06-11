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
 * @brief ND image.
 * 
 * @tparam T Element type
 * @tparam N Rank, i.e. number of axes
 * @tparam TContainer Underlying element container
 * 
 * Copy constructor and copy assignment operator perform shallow copy:
 * 
 * \code
 * auto a = Image<int, 2>("a", 4, 3).fill(1);
 * auto b = a;
 * assert(a(0, 0) == 1);
 * b.fill(2);
 * assert(a(0, 0) == 2);
 * \endcode
 * 
 * Deep copy is available as `copy_as()` or `operator+`:
 * 
 * \code
 * Image<float, 2> a("A");
 * auto b = a; // Shallow copy
 * auto c = a.copy_as("C"); // Deep copy labeled "C"
 * auto d = +a; // Deep copy labeled "copy(A)"
 * \endcode
 * 
 * By default, images may be allocated on device, e.g. GPU.
 * They can be copied to the host with `to_host()`, which is a no-op if the image is already on the host.
 */
template <typename T, int N, typename TContainer = ImageContainer<T, N>>
class Image :
    public DataMixin<T, DataArithmeticMixin<T, Image<T, N, TContainer>>, Image<T, N, TContainer>>,
    public RangeMixin<is_contiguous<TContainer>(), T, Image<T, N, TContainer>> {
public:

  static constexpr int n = N; ///< The dimension parameter
  using Container = TContainer; ///< The underlying container type
  using Shape = Position<N>; ///< The shape type
  using Domain = Box<N>; ///< The domain type

  using memory_space = typename Container::memory_space;
  using execution_space = typename Container::execution_space;

  using value_type = typename Container::value_type; ///< The raw value type
  using element_type = std::decay_t<value_type>; ///< The decayed value type
  using size_type = typename Container::size_type; ///< The index and size type
  using difference_type = std::ptrdiff_t; ///< The index difference type
  using reference = typename Container::reference_type; ///< The element reference type
  using pointer = typename Container::pointer_type; ///< The element pointer type

private:

  static constexpr int kokkos_max_dyn_rank = (n == -1 ? 7 : n); ///< The max dynamic rank supported by Kokkos
  // TODO make public variable, as well as kokkos_max_rank = 8 and kokkos_max_op_rank = 6

public:

  /**
   * @brief Constructor.
   * 
   * @param label The image label
   * @param shape The image shape along each axis
   * @param container A compatible container
   * @param args Arguments to be forwarded to the container constructor
   * @param data Some external data to be viewed as an image
   * 
   * \code
   * Image from_extents("a", width, height);
   * Image from_shape("b", a.shape());
   * Image from_pointer(Wrap(a.data()), a.shape());
   * \endcode
   */
  explicit Image(std::integral auto... shape) : Image("", shape...) {}

  /**
   * @copydoc Image()
   */
  explicit Image(const std::string& label, std::integral auto... shape) : m_container(label, shape...) {}

  /**
   * @copydoc Image()
   */
  template <std::integral TInt, typename UContainer>
  explicit Image(const Sequence<TInt, n, UContainer>& shape) : Image("", shape) // TODO use ArrayLike?
  {}

  /**
   * @copydoc Image()
   */
  template <std::integral TInt, typename UContainer>
  explicit Image(const std::string& label, const Sequence<TInt, n, UContainer>& shape) :
      Image(label, shape, std::make_index_sequence<kokkos_max_dyn_rank>()) // TODO use ArrayLike?
  {}

  /**
   * @copydoc Image()
   */
  KOKKOS_INLINE_FUNCTION explicit Image(const Container& container) : m_container(container) {}

  /**
   * @copydoc Image()
   */
  KOKKOS_INLINE_FUNCTION explicit Image(Container&& container) : m_container(LINX_FORWARD(container)) {}

  /**
   * @copydoc Image()
   */
  template <typename... TArgs>
  KOKKOS_INLINE_FUNCTION explicit Image(Forward, TArgs&&... args) : m_container(LINX_FORWARD(args)...)
  {}

  /**
   * @copydoc Image()
   */
  template <typename U>
  explicit Image(Wrap<U*> data, std::integral auto... extents) : m_container(data.value, extents...)
  {}

  /**
   * @copydoc Image()
   */
  template <typename U, std::integral TInt, typename UContainer>
  explicit Image(Wrap<U*> data, const Sequence<TInt, n, UContainer>& shape) :
      Image(data, shape, std::make_index_sequence<kokkos_max_dyn_rank>()) // TODO use ArrayLike?
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
   * @brief Image shape along all axes. 
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
    return m_container.access(0, 0, 0, 0, 0, 0, 0, 0); // TODO not scalable if max rank goes >8 some day
  }

  /**
   * @brief Address offset between the origin element (with indices 0) and the element at given indices.
   */
  KOKKOS_INLINE_FUNCTION difference_type distance_from_origin(std::integral auto... indices) const
  {
    return distance_impl(forward_as_tuple(indices...), std::make_index_sequence<sizeof...(indices)>());
  }

  /**
   * @brief Reference to the element at given indices.
   */
  KOKKOS_INLINE_FUNCTION reference operator()(std::integral auto... indices) const
  {
    return m_container(indices...);
  }

  /**
   * @brief Reference to the element at given position.
   */
  template <std::integral TInt = int, int M = n>
  KOKKOS_INLINE_FUNCTION reference operator[](const GPosition<TInt, M>& position) const // FIXME use ArrayLike?
  {
    // FIXME validate M
    return at(position, std::make_index_sequence<kokkos_max_dyn_rank>());
  }

  /**
   * @brief Get a crop of the image.
   */
  template <typename U, int M>
  auto operator[](const GBox<U, M>& region) const // not __device__ because of `region & domain()`
  {
    const auto& crop = region & domain();
    using Container = decltype(slice_all(crop, std::make_index_sequence<M>()));
    return Image<T, Container::rank(), Container>(Forward {}, slice_all(crop, std::make_index_sequence<M>()));
  }

  /**
   * @brief Get a slice of the image.
   * @param region The slicing region as a `Slice` or `Box`
   * 
   * The `Slice` must have either a rank of:
   * - 1, in which case the slicing is performed on the last axis only;
   * - `n`, in which case the slicing is performed on all axes.
   * 
   * As opposed to patches:
   * - If the slice contains singletons, the associated axes are droped;
   * - Coordinates along all axes start at index 0;
   * - The image can safely be destroyed.
   * 
   * \code
   * auto cube = Image<int, 3>(widht, height, depth);
   * auto plane = cube[Slice(0)]; // First image plane
   * auto row = cube[Slice(0)(0)]; // First image row
   * auto subcube = cube[Slice(1, 4)]; // Cube at z = 1..4
   * \endcode
   * 
   * @see where()
   */
  template <typename U, SliceType... Types>
  auto operator[](const Slice<U, Types...>& region) const // not __device__ because of `region & domain()`
  {
    const auto& crop = region & domain(); // Resolve Kokkos::ALL to drop offsets with subview
    if constexpr (sizeof...(Types) == 1) {
      using Container = decltype(slice_last(std::make_index_sequence<n - 1>(), crop));
      return Image<T, Container::rank(), Container>(Forward {}, slice_last(std::make_index_sequence<n - 1>(), crop));
    } else {
      // FIXME assert sizeoff...(Types) == n?
      using Container = decltype(slice_all(crop, std::make_index_sequence<sizeof...(Types)>()));
      return Image<T, Container::rank(), Container>(
          Forward {},
          slice_all(crop, std::make_index_sequence<sizeof...(Types)>()));
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
   * @brief Helper method to unroll indices.
   */
  template <std::size_t... Is>
  KOKKOS_INLINE_FUNCTION difference_type distance_impl(const auto& indices, std::index_sequence<Is...>) const
  {
    return ((get<Is>(indices) * m_container.stride(Is)) + ...);
  }

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
    return Kokkos::subview(m_container, get<Is>(slice).kokkos_slice()...);
  }

  /**
   * @brief Slice along the last axis.
   */
  template <typename TSlice, std::size_t... Is>
  auto slice_last(std::index_sequence<Is...>, const TSlice& slice) const
  {
    using Prepend = std::array<Kokkos::ALL_t, sizeof...(Is)>;
    return Kokkos::subview(m_container, (typename std::tuple_element<Is, Prepend>::type {})..., slice.kokkos_slice());
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

template <typename U = void, typename T, int N, typename TContainer>
auto same_layout(const std::string& label, const Image<T, N, TContainer>& in)
{
  return Image<typename Rebind<T>::As<U>, N, typename Rebind<TContainer>::As<U>>(
      Forward(),
      same_layout<U>(label, in.container()));
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

} // namespace Linx

#endif
