// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_MIXINS_RANGE_H
#define LINX_BASE_MIXINS_RANGE_H

#include "Linx/Base/Packs.h"
#include "Linx/Base/Slice.h"
#include "Linx/Base/Types.h"

#include <Kokkos_StdAlgorithms.hpp>
#include <algorithm>

namespace Linx {

namespace Impl {

template <typename TLayout>
struct IsContiguousLayout : std::false_type {};

template <>
struct IsContiguousLayout<Kokkos::LayoutLeft> : std::true_type {};

template <>
struct IsContiguousLayout<Kokkos::LayoutRight> : std::true_type {};

} // namespace Impl

template <typename TView>
constexpr bool is_contiguous()
{
  return Impl::IsContiguousLayout<typename TView::array_layout>::value;
}

/**
 * @brief Enable `Kokkos::Experimental::begin()`.
 */
template <typename TView>
auto begin(const TView& in) -> decltype(Kokkos::Experimental::begin(in))
{
  return Kokkos::Experimental::begin(in);
}

/**
 * @brief Enable `Kokkos::Experimental::end()`.
 */
template <typename TView>
auto end(const TView& in) -> decltype(Kokkos::Experimental::end(in))
{
  return Kokkos::Experimental::end(in);
}

/**
 * @brief Type with defined `begin(T)` and `end(T)` free functions in `Linx` namespace or via ADL.
 */
template <typename T>
concept Range = requires(const T& t) {
  begin(t);
  end(t);
};

/**
 * @ingroup pixelwise
 * @ingroup mixins
 * @brief Base class to provide range operations.
 * @tparam T The value type
 * @tparam TDerived The child class which implements required methods
 */
template <bool IsContiguous, typename T, typename TDerived>
struct RangeMixin {
  /**
   * @brief Test equality with values.
   */
  bool equal(std::convertible_to<T> auto... values) const
  {
    const auto& container = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), LINX_CRTP_CONST_DERIVED.base());
    return equal_impl(container, forward_as_tuple(values...), std::make_index_sequence<sizeof...(values)>());
    return LINX_CRTP_CONST_DERIVED;
  }

  /**
   * @brief Copy values.
   */
  const TDerived& assign(std::convertible_to<T> auto... values) const
  {
    const auto& container = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), LINX_CRTP_CONST_DERIVED.base());
    assign_impl(container, forward_as_tuple(values...), std::make_index_sequence<sizeof...(values)>());
    Kokkos::deep_copy(LINX_CRTP_CONST_DERIVED.base(), container);
    return LINX_CRTP_CONST_DERIVED;
  }

  /**
   * @brief Copy values from a list.
   */
  const TDerived& assign(std::initializer_list<T> values) const
  {
    return assign(values.begin());
  }

  /**
   * @brief Copy values from a range.
   */
  template <std::input_iterator TIt>
  const TDerived& assign(TIt begin) const
  {
    const auto& container = LINX_CRTP_CONST_DERIVED.base();
    auto mirror = Kokkos::create_mirror_view(container);
    auto mirror_data = mirror.data();
    Kokkos::parallel_for(
        Kokkos::RangePolicy<Kokkos::HostSpace::execution_space>(0, container.size()),
        KOKKOS_CLASS_LAMBDA(std::size_t i) { mirror_data[i] = *std::next(begin, i); });
    Kokkos::deep_copy(container, mirror);
    return LINX_CRTP_CONST_DERIVED;
  }

  /**
   * @brief Copy values from a host data pointer.
   */
  const TDerived& assign(const T* data) const
  {
    const auto& container = LINX_CRTP_CONST_DERIVED.base();
    auto mirror = Kokkos::create_mirror_view(container);
    auto mirror_data = mirror.data();
    Kokkos::parallel_for(
        Kokkos::RangePolicy<Kokkos::HostSpace::execution_space>(0, container.size()),
        KOKKOS_CLASS_LAMBDA(std::size_t i) { mirror_data[i] = data[i]; });
    Kokkos::deep_copy(container, mirror);
    return LINX_CRTP_CONST_DERIVED;
  }

  /**
   * @brief Fill the container with an arithmetic progression.
   * @param first The first value to be generated
   * @param difference The common difference
   * Conceptually, this function performs:
   * 
   * ```
   * for (int i = 0; i < size(); ++i) {
   *   (*this)[i] = first + i * difference;
   * }
   * ```
   */
  template <typename T0 = T, typename T1 = T>
  const TDerived& arithmetic(const T0& first = Limits<T0>::zero(), const T1& difference = Limits<T1>::one()) const
  {
    return generate_flat(KOKKOS_CLASS_LAMBDA(std::size_t i) { return first + i * difference; });
  }

  /**
   * @brief Fill the container with an arithmetic progression.
   * @param slice The closed generation interval
   * 
   * The resulting value of `front()` is `slice.start()`,
   * while that of `back()` is `slice.finish()`.
   */
  template <typename T0>
  const TDerived& arithmetic(const Segment<T0>& slice) const
  {
    const auto size = LINX_CRTP_CONST_DERIVED.ssize() - 1;
    const auto difference = (slice.pred().supremum - slice.pred().infimum) / size;
    return arithmetic(slice.pred().infimum, difference);
  }

  /**
   * @brief Fill the container with an arithmetic progression.
   * @param slice The generation interval, supremum of which is excluded
   * 
   * The resulting value of `front()` is `slice.start()`,
   * while that of `back()` is `slice.stop() - difference`,
   * where `difference` is the common difference between successive generated values.
   */
  template <typename T0>
  const TDerived& arithmetic(const Slice<T0>& slice) const
  {
    const auto size = LINX_CRTP_CONST_DERIVED.ssize();
    const auto difference = (slice.pred().supremum - slice.pred().infimum) / size;
    return arithmetic(slice.pred().infimum, difference);
  }

  /**
   * @brief Fill the container with a geometric progression.
   * @param first The first value to be generated
   * @param ratio The ratio between two consecutive values
   * 
   * Conceptually, this function performs:
   * 
   * ```
   * for (int i = 0; i < size(); ++i) {
   *   (*this)[i] = first * pow(ratio, i);
   * }
   * ```
   */
  template <typename T0, typename T1>
  const TDerived& geometric(const T0& first, const T1& ratio) const
  {
    return generate_flat(KOKKOS_CLASS_LAMBDA(std::size_t i) { return first * Kokkos::pow(ratio, i); });
  }

  /**
   * @brief Assign each element according to a unary generator.
   * 
   * Conceptually, this function performs:
   * 
   * ```
   * for (int i = 0; i < size(); ++i) {
   *   (*this)[i] = func(i);
   * }
   * ```
   */
  const TDerived& generate_flat(auto func) const // TODO accept args...
  {
    generate_flat_impl(func);
    return LINX_CRTP_CONST_DERIVED;
  }

  /**
   * @brief Reverse the order of the elements.
   */
  const TDerived& reverse() const // TODO to DataMixin
  {
    const auto& derived = LINX_CRTP_CONST_DERIVED;
    Kokkos::Experimental::reverse(typename TDerived::execution_space(), derived.begin(), derived.end());
    return derived;
  }

  /// @cond

  template <typename TFunc>
  void generate_flat_impl(TFunc func) const // TODO to public API, with args
  {
    const auto size = LINX_CRTP_CONST_DERIVED.size();
    const auto& derived = LINX_CRTP_CONST_DERIVED;
    using Space = typename TDerived::execution_space;
    Kokkos::parallel_for(
        "range()",
        Kokkos::RangePolicy<Space>(0, size),
        KOKKOS_CLASS_LAMBDA(std::size_t i) { derived[i] = func(i); });
  }

  /**
   * @brief Helper function for unfolding pack.
   */
  template <std::size_t... Is>
  bool equal_impl(const auto& container, const auto& values, std::index_sequence<Is...>) const
  {
    return ((container(Is) == get<Is>(values)) && ...);
  }

  /**
   * @brief Helper function for unfolding pack.
   */
  template <std::size_t... Is>
  void assign_impl(const auto& container, const auto& values, std::index_sequence<Is...>) const
  {
    ((container(Is) = get<Is>(values)), ...);
  }
  /// @endcond
};

/**
 * @brief Disable range operations for incompatible containers.
 */
template <typename T, typename TDerived>
struct RangeMixin<false, T, TDerived> {};

} // namespace Linx

#endif
