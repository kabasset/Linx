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
 * @tparam T The element type
 * @tparam N The rank, or -1 for dynamic rank
 * @tparam TContainer The underlying container type
 * 
 * Image elements are default-initialized at construction.
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

  static constexpr int max_rank = (N == -1 ? 7 : N); ///< The max dynamic rank supported by Kokkos

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
  explicit Image(std::integral auto... shape) : Image("<Image>", shape...) {}

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
  explicit Image(const Sequence<TInt, n, UContainer>& shape) : Image("<Image>", shape) // TODO use LegacyArray?
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
      Image(label, shape, std::make_index_sequence<max_rank>()) // TODO use LegacyArray?
  {}

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
      Image(data, shape, std::make_index_sequence<max_rank>()) // TODO use LegacyArray?
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
   * Therefore, `data()` can be less than `&front()`,
   * typically for memory alignment or padding purposes.
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
    static_assert(max_rank <= 8);
    return m_container.access(0, 0, 0, 0, 0, 0, 0, 0);
  }

  /**
   * @brief Access the last element.
   */
  KOKKOS_INLINE_FUNCTION reference back() const
  {
    static_assert(max_rank <= 8);
    Kokkos::Array<int, 8> p;
    for (int i = 0; i < rank(); ++i) {
      p[i] = extent(i) - 1;
    }
    return m_container.access(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
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
  [[deprecated]] KOKKOS_INLINE_FUNCTION reference
  operator[](const GPosition<TInt, M>& position) const // FIXME use LegacyArray?
  {
    return at_impl(position, std::make_index_sequence<max_rank>());
  }

  /**
   * @brief Access the element at given position, with bounds checking and support for backward indexing.
   */
  KOKKOS_INLINE_FUNCTION reference at(std::integral auto... position) const // TODO to DataMixin
  {
    return at(std::array {position...}); // FIXME avoid copy
  }

  /**
   * @brief Access the element at given position, with bounds checking and support for backward indexing.
   */
  KOKKOS_INLINE_FUNCTION reference at(const LegacyArray auto& position) const // TODO to DataMixin
  {
    return at_impl(position, std::make_index_sequence<max_rank>());
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
  template <std::integral TInt, typename... TFuncs>
  auto operator[](const Slice<TInt, TFuncs...>& region) const // not __device__ because of `region & domain()`
  {
    const auto& crop = region & domain(); // Resolve Kokkos::ALL to drop offsets with subview
    if constexpr (sizeof...(TFuncs) == 1) {
      using Container = decltype(slice_last(std::make_index_sequence<n - 1>(), crop));
      return Image<T, Container::rank(), Container>(Forward {}, slice_last(std::make_index_sequence<n - 1>(), crop));
    } else {
      static_assert(sizeof...(TFuncs) == n);
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
  KOKKOS_INLINE_FUNCTION reference at_impl(const TPosition& position, std::index_sequence<Is...>) const
  {
    return operator()(index_along<Is>(get_or<Is>(position, 0))...);
  }

  /**
   * @brief Deduce the in-bounds index, with support for backward indexing.
   */
  template <int I>
  KOKKOS_INLINE_FUNCTION auto index_along(std::integral auto i) const
  {
    const auto stop = extent(I);
    const auto out = i < 0 ? stop - i : i;
    OutOfBounds::may_abort("index", out, Slice(0, stop));
    return out;
  }

  /**
   * @brief Helper function for 0-based fixed-rank containers.
   */
  template <typename... TArgs>
  static Domain domain(const Kokkos::View<TArgs...>& container) // TODO free function
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
  static Domain domain(const Kokkos::DynRankView<TArgs...>& container) // TODO free function
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

} // namespace Linx

// aliases and deduction guides
#include "Linx/Data/Image/types.h"
// creation functions
#include "Linx/Data/Image/creation.h"
// other free functions
#include "Linx/Data/Image/funcs.h"

#endif
