// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_IMAGE_H
#define LINX_DATA_IMAGE_H

#include "Linx/Base/Functional.h"
#include "Linx/Base/Slice.h"
#include "Linx/Base/Types.h"
#include "Linx/Base/Views.h"
#include "Linx/Base/mixins/Data.h"
#include "Linx/Base/mixins/Range.h"
#include "Linx/Data/Box.h"

#include <Kokkos_Core.hpp>
#include <concepts>
#include <string>

namespace Linx {

/**
 * @ingroup arrays
 * @brief Non-resizable ND array.
 * 
 * @tparam T The element type
 * @tparam TDomain The domain type
 * @tparam TView The underlying view type
 * 
 * The image domain is a box, which does not necessarily starts at position 0,
 * and can have static or dynamic bounds.
 * 
 * Copy constructor and copy assignment operator perform shallow copy.
 * 
 * Creation functions (@ref creation) make code much less verbose and should be preferred to constructors.
 * 
 * @see arrays
 * @see `Box`
 * @see `DataMixin`
 * @see `RangeMixin`
 */
template <typename T, typename TDomain, typename TView = ImageView<T, TDomain>>
class Image :
    public DataMixin<T, DataArithmeticMixin<T, Image<T, TDomain, TView>>, Image<T, TDomain, TView>>,
    public RangeMixin<is_contiguous<TView>(), T, Image<T, TDomain, TView>> { // FIXME bool(Range<TView>)
public:

  using element_type = T; ///< The possibly const-qualified element type
  using value_type = std::remove_cvref_t<element_type>; ///< The value type
  using reference = T&; ///< The value reference type
  using const_reference = const T&; ///< The constant reference type
  using pointer = T*; ///< The value pointer type
  using const_pointer = const T*; ///< The constant pointer type

  using Domain = TDomain; ///< The domain type // FIXME domain_type
  using Shape = typename Domain::value_type; ///< The shape type ///< FIXME shape_type?
  static constexpr int n = Domain::n; ///< The rank parameter
  static constexpr int max_rank = (n == -1 ? kokkos_max_dyn_rank : n); ///< The max rank supported by Kokkos
  static constexpr bool static_rank_flag = (n >= 0); ///< Static rank flag
  static constexpr bool static_domain_flag = Domain::static_flag; ///< Static domain flag
  static constexpr bool static_start_at_origin_flag = Domain::static_start_at_origin_flag; ///< Shape-only domain flag
  static constexpr bool static_contiguous_flag = is_contiguous<TView>(); ///< Contiguity flag

  using base_type = TView; ///< The underlying view type
  using memory_space = typename base_type::memory_space; ///< The memory space
  using execution_space = typename base_type::execution_space; ///< The default execution space

  /**
   * @brief Constructor.
   * 
   * @param shape The image extents along each axis
   * 
   * @warning If the rank is static, the extent count must match it.
   */
  explicit Image(std::integral auto... shape) : Image("<Image>", shape...) {}

  /**
   * @brief Constructor.
   * 
   * @param label The image label
   * @param shape The image extent along each axis
   * 
   * @warning If the rank is static, the extent count must match it.
   */
  explicit Image(const std::string& label, std::integral auto... shape) : m_view(label, shape...), m_domain {}
  {
    if constexpr (not static_start_at_origin_flag) {
      throw std::runtime_error("Image(label, shape): domain is missing.");
    }
  }

  /**
   * @brief Constructor.
   * 
   * @param domain The image domain
   */
  explicit Image(const Domain& domain) : Image("<Image>", domain) {}

  /**
   * @brief Constructor.
   * 
   * @param label The image label
   * @param domain The image domain
   */
  explicit Image(const std::string& label, const Domain& domain) :
      Image(label, domain, std::make_index_sequence<max_rank>())
  {}

  /**
   * @brief Forwarding constructor.
   * @param args The arguments to be forwarded to the view's constructor
   */
  explicit Image(Forward, auto&&... args) : m_view(LINX_FORWARD(args)...), m_domain {}
  {
    if constexpr (not static_start_at_origin_flag) {
      throw std::runtime_error("Image(Forward, args): domain is missing.");
    }
  }

  /**
   * @brief Forwarding constructor.
   * @param domain The image domain
   * @param args The arguments to be forwarded to the view's constructor
   */
  explicit Image(const Domain& domain, Forward, auto&&... args) : m_view(LINX_FORWARD(args)...), m_domain {domain} {}

  /**
   * @brief Wrapping constructor.
   * @param data The wrapped data
   * @param shape The image extents along each axis
   * 
   * The resulting image does not own the data.
   * It won't manage its memory or ensure it is valid.
   * 
   * @warning If the rank is static, the extent count must match it.
   */
  template <typename TValue>
  explicit Image(Wrap<TValue*> data, std::integral auto... shape) : m_view(data.value, shape...), m_domain {}
  {
    if constexpr (not static_start_at_origin_flag) {
      throw std::runtime_error("Image(data, shape): domain is missing.");
    }
  }

  /**
   * @brief Wrapping constructor.
   * @param data The wrapped data
   * @param domain The image domain
   * 
   * The resulting image does not own the data.
   * It won't manage its memory or ensure it is valid.
   */
  template <typename TValue>
  explicit Image(Wrap<TValue*> data, const Domain& domain) : Image(data, domain, std::make_index_sequence<max_rank>())
  {}

  /**
   * @brief Image rank.
   */
  KOKKOS_INLINE_FUNCTION int rank() const
  {
    if constexpr (n == -1) {
      return Kokkos::rank(m_view);
    } else {
      return n;
    }
  }

  /**
   * @brief Start index along given axis.
   */
  KOKKOS_INLINE_FUNCTION auto start(std::integral auto i) const
  {
    if constexpr (static_start_at_origin_flag) {
      return 0;
    } else {
      return m_domain.start(i);
    }
  }

  /**
   * @brief Stop index along given axis.
   */
  KOKKOS_INLINE_FUNCTION auto stop(std::integral auto i) const
  {
    if constexpr (static_start_at_origin_flag) {
      return extent(i);
    } else {
      return m_domain.stop(i);
    }
  }

  /**
   * @brief Image extent along a given axis.
   */
  KOKKOS_INLINE_FUNCTION int extent(std::integral auto i) const
  {
    return m_view.extent_int(i);
  }

  /**
   * @brief Image extents along all axes. 
   */
  Shape shape() const // FIXME rm?
  {
    if constexpr (static_start_at_origin_flag) {
      auto out = std::vector<int>(rank());
      for (int i = 0; i < rank(); ++i) {
        out[i] = m_view.extent_int(i);
      }
      return Shape(out.begin(), out.end());
    } else {
      return domain().shape();
    }
  }

  /**
   * @brief Image domain.
   */
  decltype(auto) domain() const
  {
    if constexpr (static_start_at_origin_flag) {
      return domain_impl(m_view);
    } else {
      return m_domain;
    }
  }

  /**
   * @brief Memory stride along given axis.
   */
  KOKKOS_INLINE_FUNCTION auto stride(std::integral auto i) const
  {
    return m_view.stride(i);
  }

  /**
   * @brief Memory striding.
   */
  auto strides() const
  {
    std::vector<Index> longer(rank() + 1);
    m_view.stride(longer.data());
    return Vector<Index*>(longer.data(), longer.data() + rank());
  }

  /**
   * @brief Underlying view.
   */
  KOKKOS_INLINE_FUNCTION const base_type& base() const // FIXME base()
  {
    return m_view;
  }

  /**
   * @brief Access the first element.
   * 
   * As opposed to `data()`, which is the pointer to the allocated memory,
   * `&front()` is a pointer to the first element.
   * Therefore, `data()` can be less than `&front()`,
   * typically for memory alignment or padding purposes.
   * 
   * If the domain does not start at position 0, then `front()` and `origin()` are different.
   * 
   * @see `origin()`
   */
  KOKKOS_INLINE_FUNCTION reference front() const
  {
    static_assert(max_rank <= 8);
    return m_view.access(0, 0, 0, 0, 0, 0, 0, 0);
  }

  /**
   * @brief Access the element at position 0.
   */
  KOKKOS_INLINE_FUNCTION reference origin() const // FIXME return pointer?
  {
    if constexpr (static_start_at_origin_flag) {
      return front();
    } else {
      return at_impl<false>(vec(), std::make_index_sequence<max_rank>());
    }
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
    return m_view.access(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
  }

  /**
   * @brief Access the element at given position.
   */
  KOKKOS_INLINE_FUNCTION reference operator()(std::integral auto... position) const
  {
    return at_impl<false>(vec(position...), std::make_index_sequence<max_rank>()); // FIXME avoid vec()
  }

  /**
   * @brief Access the element at given position, with bounds checking.
   */
  KOKKOS_INLINE_FUNCTION reference at(std::integral auto... position) const // TODO to DataMixin?
  {
    return at(vec(position...)); // FIXME avoid vec()
  }

  /**
   * @brief Access the element at given position, with bounds checking.
   */
  KOKKOS_INLINE_FUNCTION reference at(const auto& position) const // TODO to DataMixin?
  {
    return at_impl(position, std::make_index_sequence<max_rank>());
  }

  /**
   * @brief Access the i-th element in the storage order.
   */
  KOKKOS_INLINE_FUNCTION reference operator[](std::integral auto i) const
  {
    return m_view.accessor().access(m_view.data_handle(), i);
    // FIXME check that this undocumented API is stable (same as mdspan)
  }

  /**
   * @brief Slice the image as a shallow copy.
   * @param region The slicing as a `Box`
   */
  template <typename TStart, typename TStop>
  auto operator[](const Box<TStart, TStop>& region) const // not __device__ because of `region & domain()`
  {
    const auto& crop = region & domain();
    using Crop = LINX_DECLTYPE(crop);
    using View = LINX_DECLTYPE(slice_all(crop, std::make_index_sequence<Crop::n>()));
    using Domain = LINX_DECLTYPE(domain(std::declval<View>()));
    return Image<T, Domain, View>(Forward {}, slice_all(crop, std::make_index_sequence<Crop::n>()));
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
      using View = LINX_DECLTYPE(slice_last(std::make_index_sequence<n - 1>(), crop));
      using Domain = LINX_DECLTYPE(domain(std::declval<View>()));
      return Image<T, Domain, View>(Forward {}, slice_last(std::make_index_sequence<n - 1>(), crop));
    } else {
      static_assert(sizeof...(TFuncs) == n);
      using View = LINX_DECLTYPE(slice_all(crop, std::make_index_sequence<sizeof...(TFuncs)>()));
      using Domain = LINX_DECLTYPE(domain(std::declval<View>()));
      return Image<T, Domain, View>(Forward {}, slice_all(crop, std::make_index_sequence<sizeof...(TFuncs)>()));
    }
    // FIXME offset
  }

private:

  /**
   * @brief Helper constructor to unroll shape.
   */
  template <std::size_t... Is>
  Image(const std::string& label, const Domain& domain, std::index_sequence<Is...>) :
      m_view(label, get_or<Is, KOKKOS_INVALID_INDEX>(domain.shape())...),
      m_domain {domain}
  {}

  /**
   * @brief Helper constructor to unroll shape.
   */
  template <typename TValue, std::size_t... Is>
  Image(Wrap<TValue*> data, const Domain& domain, std::index_sequence<Is...>) :
      m_view(data.value, get_or<Is, KOKKOS_INVALID_INDEX>(domain.shape())...),
      m_domain {domain}
  {}

  /**
   * @brief Helper accessor to unroll position.
   */
  template <bool CheckBounds = true, typename TPosition, std::size_t... Is>
  KOKKOS_INLINE_FUNCTION reference at_impl(const TPosition& position, std::index_sequence<Is...>) const
  {
    return m_view.access(index_along<Is, CheckBounds>(get_or<Is, 0>(position))...);
  }

  /**
   * @brief Underlying index.
   */
  template <int I, bool CheckBounds>
  KOKKOS_INLINE_FUNCTION auto index_along(std::integral auto i) const
  {
    if constexpr (CheckBounds) {
      OutOfBounds::may_abort("index", i, Slice(start(I), stop(I)));
    }
    if constexpr (static_start_at_origin_flag) {
      return i;
    } else {
      return i - m_domain.start(I);
    }
  }

  /**
   * @brief Helper function for 0-based fixed-rank views.
   */
  template <typename... TArgs>
  static auto domain_impl(const auto& view)
  {
    if constexpr (static_domain_flag) {
      return Domain();
    } else {
      const auto& box = domain(view); // Not necessarily of type Domain
      return Domain(box.start(), box.stop());
    }
  }

  template <typename... TArgs>
  static auto domain(const Kokkos::View<TArgs...>& view)
  { // TODO free function
    static constexpr auto n = Kokkos::View<TArgs...>::rank();
    auto stop = vec<Dimension {n}>(0);
    for (std::size_t i = 0; i < n; ++i) {
      stop[i] = view.extent_int(i);
    }
    return Box(stop);
  }

  /**
   * @brief Helper function for 0-based dynamic rank views.
   */
  template <typename... TArgs>
  static auto domain(const Kokkos::DynRankView<TArgs...>& view) // TODO free function
  {
    auto rank = view.rank();
    auto stop = vec(Dimension {rank}, 0);
    for (LINX_DECLTYPE(rank) i = 0; i < rank; ++i) {
      stop[i] = view.extent_int(i);
    }
    return Box(stop);
  }

  /**
   * @brief Slice along each axis.
   */
  template <typename TSlice, std::size_t... Is>
  auto slice_all(const TSlice& slice, std::index_sequence<Is...>) const
  {
    return Kokkos::subview(m_view, kokkos_slice(get<Is>(slice))...);
    // FIXME offset
  }

  /**
   * @brief Slice along the last axis.
   */
  template <typename TSlice, std::size_t... Is>
  auto slice_last(std::index_sequence<Is...>, const TSlice& slice) const
  {
    using Prepend = std::array<Kokkos::ALL_t, sizeof...(Is)>;
    return Kokkos::subview(m_view, (typename std::tuple_element<Is, Prepend>::type {})..., kokkos_slice(slice));
    // FIXME offset
  }

private:

  base_type m_view; ///< The underlying view
  using DummyDomain = Vector<>; ///< Placeholder with dummy variadic ctor
  std::conditional_t<static_start_at_origin_flag, DummyDomain, Domain> m_domain; ///< The domain, if any
};

} // namespace Linx

#include "Linx/Data/Image/creation.h"
#include "Linx/Data/Image/funcs.h"
#include "Linx/Data/Image/types.h"

#endif
