// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_SEQUENCE_H
#define LINX_DATA_SEQUENCE_H

#include "Linx/Base/Containers.h"
#include "Linx/Base/Functional.h"
#include "Linx/Base/Slice.h"
#include "Linx/Base/Types.h"
#include "Linx/Base/concepts/Array.h"
#include "Linx/Base/mixins/Data.h"
#include "Linx/Base/mixins/Range.h"

#include <Kokkos_Core.hpp>
#include <Kokkos_StdAlgorithms.hpp>
#include <concepts>
#include <ranges>
#include <string>

namespace Linx {

template <typename T, int N, typename TContainer = SequenceContainer<T, N>>
using Sequence = Image<T, Shape<std::integer_sequence<int, N>>, TContainer>;

namespace Impl {

template <typename... TArgs>
struct RangeTraits;

template <>
struct RangeTraits<> {
  using value_type = int;
};

template <typename T>
struct RangeTraits<T> {
  using value_type = T;
};

template <typename T0, std::convertible_to<T0> T1>
struct RangeTraits<T0, T1> {
  using value_type = std::common_type_t<T0, T1>;
};

template <typename T, typename TPred>
struct RangeTraits<Slice<T, TPred>> {
  using value_type = T;
};

template <typename TStart, typename TStep>
struct RangeTraits<TStart, TStep> {
  using value_type = std::common_type_t<TStart, typename TStep::value_type>;
};

} // namespace Impl

/**
 * @ingroup creation
 * @brief Static-size arithmetic sequence.
 * @tparam N The static size
 * @tparam TSpace The memory space
 * @param label The sequence label
 * @param args A slice, or a first value and common difference.
 * 
 * @see `RangeMixin::arithmetic()`
 */
template <int N, typename TSpace = Kokkos::DefaultExecutionSpace, typename... TArgs>
auto arithmetic(const std::string& label, TArgs&&... args)
{
  using T = Impl::RangeTraits<TArgs...>::value_type;
  return Linx::Sequence<T, N, SequenceContainer<T, N, TSpace>>(label).arithmetic(LINX_FORWARD(args)...);
}

/**
 * @ingroup creation
 * @brief Dynamic-size arithmetic sequence.
 * @tparam TSpace The memory space
 * @param size The dynamic size
 * @param label The sequence label
 * @param args A slice, or a start value and step.
 * 
 * @see `RangeMixin::arithmetic()`
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename... TArgs>
auto arithmetic(std::integral auto size, const std::string& label, TArgs&&... args)
{
  using T = Impl::RangeTraits<TArgs...>::value_type;
  return Linx::Sequence<T, -1, SequenceContainer<T, -1, TSpace>>(label, size).arithmetic(LINX_FORWARD(args)...);
}

/**
 * @ingroup geometric
 * @brief Static-size geometric sequence.
 * @tparam N The static size
 * @tparam TSpace The memory space
 * @param label The sequence label
 * @param args A slice, or a first value and common ratio.
 * 
 * @see `RangeMixin::geometric()`
 */
template <int N, typename TSpace = Kokkos::DefaultExecutionSpace, typename... TArgs>
auto geometric(const std::string& label, TArgs&&... args)
{
  using T = Impl::RangeTraits<TArgs...>::value_type;
  return Linx::Sequence<T, N, SequenceContainer<T, N, TSpace>>(label).geometric(LINX_FORWARD(args)...);
}

/**
 * @ingroup geometric
 * @brief Dynamic-size geometric sequence.
 * @tparam TSpace The memory space
 * @param size The dynamic size
 * @param label The sequence label
 * @param args A slice, or a first value and common ratio.
 * 
 * @see `RangeMixin::geometric()`
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename... TArgs>
auto geometric(std::integral auto size, const std::string& label, TArgs&&... args)
{
  using T = Impl::RangeTraits<TArgs...>::value_type;
  return Linx::Sequence<T, -1, SequenceContainer<T, -1, TSpace>>(label, size).geometric(LINX_FORWARD(args)...);
}

/**
 * @ingroup creation
 * @brief Generate a static-size sequence.
 * @tparam N The size
 * @tparam TSpace The memory space
 * @param label The label
 * @param func The generator
 */
template <int N, typename TSpace = Kokkos::DefaultExecutionSpace, typename TFunc>
auto generate(const std::string& label, const TFunc& func)
{
  static_assert(N >= 0);
  using T = std::remove_cvref_t<decltype(func(0))>;
  return Sequence<T, N, SequenceContainer<T, N, TSpace>>(label).copy_from(func); // TODO uninitialized
}

/**
 * @ingroup creation
 * @brief Generate a dynamic-size sequence.
 * @tparam TSpace The memory space
 * @param size The size
 * @param label The label
 * @param func The generator
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename TFunc>
auto generate(std::integral auto size, const std::string& label, const TFunc& func)
{
  using T = std::remove_cvref_t<decltype(func(0))>;
  return Sequence<T, -1, SequenceContainer<T, -1, TSpace>>(label, size).copy_from(func); // TODO uninitialized
}

/**
 * @ingroup creation
 * @brief Copy an array as a static-size sequence.
 * @tparam N The static size
 * @tparam TSpace The memory space
 * @param label The sequence label
 * @param in The input array
 * 
 * If the input array is larger than `N`, only the `N` first values are copied.
 * If it is smaller, the remaining values are default-initialized.
 */
template <int N, typename TSpace = Kokkos::DefaultExecutionSpace>
auto resize(const std::string& label, const LegacyArray auto& in)
{
  static_assert(N >= 0);
  using T = std::remove_cvref_t<decltype(in[0])>;
  Sequence<T, N, SequenceContainer<T, N, TSpace>> out(label);
  copy_to(in, out);
  return out;
}

/**
 * @ingroup creation
 * @brief Copy an array as a dynamic-size sequence.
 * @tparam TSpace The memory space
 * @param size The dynamic size
 * @param label The sequence label
 * @param in The input array
 * 
 * If the input array is larger than `size`, only the `size` first values are copied.
 * If it is smaller, the remaining values are default-initialized.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
auto resize(std::integral auto size, const std::string& label, const LegacyArray auto& in)
{
  using T = std::remove_cvref_t<decltype(in[0])>;
  Sequence<T, -1, SequenceContainer<T, -1, TSpace>> out(label, size);
  copy_to(in, out);
  return out;
}

/**
 * @ingroup creation
 * @brief Copy a list as a static-size sequence.
 * @tparam N The static size
 * @tparam TSpace The memory space
 * @param label The sequence label
 * @param in The input list
 * 
 * If the input list is larger than `N`, only the `N` first values are copied.
 * If it is smaller, the remaining values are default-initialized.
 */
template <int N, typename TSpace = Kokkos::DefaultExecutionSpace, typename T>
auto resize(const std::string& label, std::initializer_list<T> in)
{
  return resize<N, TSpace>(label, Sequence<T, -1, SequenceContainer<T, -1, TSpace>>(in));
}

/**
 * @ingroup creation
 * @brief Copy a list as a dynamic-size sequence.
 * @tparam TSpace The memory space
 * @param size The dynamic size
 * @param label The sequence label
 * @param in The input list
 * 
 * If the input list is larger than `size`, only the `size` first values are copied.
 * If it is smaller, the remaining values are default-initialized.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T>
auto resize(std::integral auto size, const std::string& label, std::initializer_list<T> in)
{
  return resize<TSpace>(size, label, Sequence<T, -1, SequenceContainer<T, -1, TSpace>>(in));
}

} // namespace Linx

#endif
