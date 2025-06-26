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

template <typename TContainer>
constexpr bool is_contiguous()
{
  return Impl::IsContiguousLayout<typename TContainer::array_layout>::value;
}

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
  KOKKOS_INLINE_FUNCTION bool equal(std::convertible_to<T> auto... values) const
  {
    return equal_impl(forward_as_tuple(values...), std::make_index_sequence<sizeof...(values)>());
  }

  /**
   * @brief Copy values.
   */
  KOKKOS_INLINE_FUNCTION const TDerived& assign(std::convertible_to<T> auto... values) const
  {
    assign_impl(forward_as_tuple(values...), std::make_index_sequence<sizeof...(values)>());
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
    const auto& container = LINX_CRTP_CONST_DERIVED.container();
    auto mirror = Kokkos::create_mirror_view(container);
    auto mirror_data = mirror.data();
    Kokkos::parallel_for(
        Kokkos::RangePolicy<Kokkos::HostSpace::execution_space>(0, container.size()),
        KOKKOS_LAMBDA(int i) { mirror_data[i] = *std::next(begin, i); });
    Kokkos::deep_copy(container, mirror);
    return LINX_CRTP_CONST_DERIVED;
  }

  /**
   * @brief Copy values from a host data pointer.
   */
  const TDerived& assign(const T* data) const
  {
    const auto& container = LINX_CRTP_CONST_DERIVED.container();
    auto mirror = Kokkos::create_mirror_view(container);
    auto mirror_data = mirror.data();
    Kokkos::parallel_for(
        Kokkos::RangePolicy<Kokkos::HostSpace::execution_space>(0, container.size()),
        KOKKOS_LAMBDA(int i) { mirror_data[i] = data[i]; });
    Kokkos::deep_copy(container, mirror);
    return LINX_CRTP_CONST_DERIVED;
  }

  /**
   * @brief Fill the container with an arithmetic progression.
   * @param slice The closed generation interval
   * 
   * The resulting value of `front()` is `slice.start()`,
   * while that of `back()` is `slice.stop()`.
   */
  template <typename T0>
  const TDerived& arithmetic(const Segment<T0>& slice) const
  {
    const auto size = LINX_CRTP_CONST_DERIVED.ssize() - 1;
    const auto step = (slice.pred().supremum - slice.pred().infimum) / size;
    return arithmetic(slice.pred().infimum, Add(step));
  }

  /**
   * @brief Fill the container with an arithmetic progression.
   * @param slice The generation interval, supremum of which is excluded
   * 
   * The resulting value of `front()` is `slice.start()`,
   * while that of `back()` is `slice.stop() - step`,
   * where `step` is the difference between successive generated values.
   */
  template <typename T0>
  const TDerived& arithmetic(const Span<T0>& slice) const
  {
    const auto size = LINX_CRTP_CONST_DERIVED.ssize();
    const auto step = (slice.pred().supremum - slice.pred().infimum) / size;
    return arithmetic(slice.pred().infimum, Add(step));
  }

  /**
   * @brief Fill the container with an arithmetic progression.
   * @param start The first value to be generated
   * @param step The difference between two consecutive values
   * Conceptually, this function performs:
   * 
   * ```
   * for (int i = 0; i < size(); ++i) {
   *   (*this)[i] = start + i * step;
   * }
   * ```
   */
  template <typename T0 = T, typename T1 = T>
  const TDerived&
  arithmetic(const T0& start = Limits<T0>::zero(), const Add<Forward, T1>& step = Limits<T1>::one()) const
  {
    return generate_flat(KOKKOS_LAMBDA(int i) { return start + i * step.rhs; });
  }

  /**
   * @brief Fill the container with an arithmetic progression.
   * @param start The first value to be generated
   * @param step The opposite of the difference between two consecutive values
   * 
   * Conceptually, this function performs:
   * 
   * ```
   * for (int i = 0; i < size(); ++i) {
   *   (*this)[i] = start - i * step;
   * }
   * ```
   */
  template <typename T0, typename T1>
  const TDerived& arithmetic(const T0& start, const Subtract<Forward, T1>& step) const
  {
    return generate_flat(KOKKOS_LAMBDA(int i) { return start - i * step.rhs; });
  }

  /**
   * @brief Fill the container with a geometric progression.
   * @param start The first value to be generated
   * @param step The ratio between two consecutive values
   * 
   * Conceptually, this function performs:
   * 
   * ```
   * for (int i = 0; i < size(); ++i) {
   *   (*this)[i] = start * pow(step, i);
   * }
   * ```
   */
  template <typename T0, typename T1>
  const TDerived& geometric(const T0& start, const Multiply<Forward, T1>& step) const
  {
    return generate_flat(KOKKOS_LAMBDA(int i) { return start * Kokkos::pow(step.rhs, i); });
  }

  /**
   * @brief Fill the container with a geometric progression.
   * @param start The first value to be generated
   * @param step The inverse of the ratio between two consecutive values
   * 
   * Conceptually, this function performs:
   * 
   * ```
   * for (int i = 0; i < size(); ++i) {
   *   (*this)[i] = start * pow(step, -i);
   * }
   * ```
   */
  template <typename T0, typename T1>
  const TDerived& geometric(const T0& start, const Divide<Forward, T1>& step) const
  {
    return generate_flat(KOKKOS_LAMBDA(int i) { return start * Kokkos::pow(step.rhs, -i); });
  }

  /**
   * @brief Assign each element according to a monadic generator.
   * 
   * Conceptually, this function performs:
   * 
   * ```
   * for (int i = 0; i < size(); ++i) {
   *   (*this)[i] = func(i);
   * }
   * ```
   */
  const TDerived& generate_flat(auto func) const // FIXME args...
  {
    generate_flat_impl(func);
    return LINX_CRTP_CONST_DERIVED;
  }

  /**
   * @brief Reference to the i-th element.
   */
  KOKKOS_INLINE_FUNCTION auto& operator[](std::integral auto i) const
  {
    // return *std::ranges::next(std::ranges::begin(LINX_CRTP_CONST_DERIVED), i); // device-incompatible
    auto ptr = &LINX_CRTP_CONST_DERIVED.origin();
    return ptr[i]; // FIXME not necessarily contiguous => * stride(0)?
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
    auto ptr = &LINX_CRTP_CONST_DERIVED.origin();
    const auto size = LINX_CRTP_CONST_DERIVED.size();
    using Space = typename TDerived::execution_space;
    Kokkos::parallel_for("range()", Kokkos::RangePolicy<Space>(0, size), KOKKOS_LAMBDA(int i) { ptr[i] = func(i); });
    // FIXME what if stride(0) != 1 ?
  }

  template <std::size_t... Is>
  KOKKOS_INLINE_FUNCTION bool equal_impl(const auto& values, std::index_sequence<Is...>) const
  {
    const auto& container = LINX_CRTP_CONST_DERIVED.container(); // FIXME enable on device
    return ((container(Is) == get<Is>(values)) && ...);
  }

  /**
   * @brief Helper function for unfolding pack.
   */
  template <std::size_t... Is>
  KOKKOS_INLINE_FUNCTION void assign_impl(const auto& values, std::index_sequence<Is...>) const
  {
    const auto& container = LINX_CRTP_CONST_DERIVED.container(); // FIXME enable on device
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
