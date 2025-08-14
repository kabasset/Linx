// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_SEQUENCE_CREATION_H
#define LINX_DATA_SEQUENCE_CREATION_H

#include "Linx/Data/Image/creation.h"

#include <Kokkos_Core.hpp>

namespace Linx {

/**
 * @ingroup creation
 * @brief Create a static-size, default-initialized sequence.
 * @tparam T The value type
 * @tparam N The size
 * @tparam TSpace The memory space
 * @param label The label
 */
template <typename T, std::size_t N, typename TSpace = Kokkos::DefaultExecutionSpace>
auto default_init(const std::string& label)
{
  return default_init<T, TSpace>(label, shape<N>());
}

/**
 * @ingroup creation
 * @brief Create a dynamic-size, default-initialized sequence.
 * @tparam T The value type
 * @tparam TSpace The memory space
 * @param size The size
 * @param label The label
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
auto default_init(std::size_t size, const std::string& label)
{
  return default_init<T, TSpace>(label, size);
}

/**
 * @ingroup creation
 * @brief Create a static-size, uninitialized sequence.
 * @tparam T The value type
 * @tparam N The size
 * @tparam TSpace The memory space
 * @param label The label
 */
template <typename T, std::size_t N, typename TSpace = Kokkos::DefaultExecutionSpace>
auto no_init(const std::string& label)
{
  return no_init<T, TSpace>(label, shape<N>());
}

/**
 * @ingroup creation
 * @brief Create a dynamic-size, uninitialized sequence.
 * @tparam T The value type
 * @tparam TSpace The memory space
 * @param size The size
 * @param label The label
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
auto no_init(std::size_t size, const std::string& label)
{
  return no_init<T, TSpace>(label, size);
}

/**
 * @ingroup creation
 * @brief Create a static-size sequence from a pack of values.
 * @tparam TSpace The memory space
 * @param label The label
 * @param value0, values The values
 * 
 * The type of the first value is used as the sequence value type.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T0, std::convertible_to<T0>... Ts>
auto seq(const std::string& label, T0 value0, Ts... values)
{
  return no_init<T0, 1 + sizeof...(Ts), TSpace>(label).assign(value0, values...);
}

/**
 * @ingroup creation
 * @brief Create a static-size sequence filled with a single value.
 * @tparam N The size
 * @tparam TSpace The memory space
 * @param label The label
 * @param value The value
 */
template <std::size_t N, typename TSpace = Kokkos::DefaultExecutionSpace, typename T>
auto fill(const std::string& label, const T& value)
{
  return fill<TSpace>(label, value, shape<N>());
}

/**
 * @ingroup creation
 * @brief Create a dynamic-size sequence filled with a single value.
 * @tparam TSpace The memory space
 * @param size The size
 * @param label The label
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
auto fill(std::size_t size, const std::string& label, auto value)
{
  return fill<TSpace>(label, value, size);
}

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
 * @brief Generate a static-size sequence.
 * @tparam N The size
 * @tparam TSpace The memory space
 * @param label The label
 * @param func The generator
 */
template <std::size_t N, typename TSpace = Kokkos::DefaultExecutionSpace>
auto generate(const std::string& label, const auto& func)
{
  return generate<TSpace>(label, func, shape<N>());
}

/**
 * @ingroup creation
 * @brief Generate a dynamic-size sequence.
 * @tparam TSpace The memory space
 * @param size The size
 * @param label The label
 * @param func The generator
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
auto generate(std::size_t size, const std::string& label, const auto& func)
{
  return generate<TSpace>(label, func, size);
}

/**
 * @ingroup creation
 * @brief Static-size arithmetic sequence.
 * @tparam N The size
 * @tparam TSpace The memory space
 * @param label The label
 * @param args A slice, or a first value and common difference.
 * 
 * @see `RangeMixin::arithmetic()`
 */
template <std::size_t N, typename TSpace = Kokkos::DefaultExecutionSpace, typename... TArgs>
auto arithmetic(const std::string& label, TArgs&&... args)
{
  using T = Impl::RangeTraits<TArgs...>::value_type;
  return Linx::no_init<T, N, TSpace>(label).arithmetic(LINX_FORWARD(args)...);
}

/**
 * @ingroup creation
 * @brief Dynamic-size arithmetic sequence.
 * @tparam TSpace The memory space
 * @param size The size
 * @param label The label
 * @param args A slice, or a start value and step.
 * 
 * @see `RangeMixin::arithmetic()`
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename... TArgs>
auto arithmetic(std::size_t size, const std::string& label, TArgs&&... args)
{
  using T = Impl::RangeTraits<TArgs...>::value_type;
  return Linx::no_init<T, TSpace>(size, label).arithmetic(LINX_FORWARD(args)...);
}

/**
 * @ingroup geometric
 * @brief Static-size geometric sequence.
 * @tparam N The size
 * @tparam TSpace The memory space
 * @param label The label
 * @param args A slice, or a first value and common ratio.
 * 
 * @see `RangeMixin::geometric()`
 */
template <std::size_t N, typename TSpace = Kokkos::DefaultExecutionSpace, typename... TArgs>
auto geometric(const std::string& label, TArgs&&... args)
{
  using T = Impl::RangeTraits<TArgs...>::value_type;
  return Linx::no_init<T, N, TSpace>(label).geometric(LINX_FORWARD(args)...);
}

/**
 * @ingroup geometric
 * @brief Dynamic-size geometric sequence.
 * @tparam TSpace The memory space
 * @param size The size
 * @param label The label
 * @param args A slice, or a first value and common ratio.
 * 
 * @see `RangeMixin::geometric()`
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename... TArgs>
auto geometric(std::size_t size, const std::string& label, TArgs&&... args)
{
  using T = Impl::RangeTraits<TArgs...>::value_type;
  return Linx::no_init<T, TSpace>(size, label).geometric(LINX_FORWARD(args)...);
}

} // namespace Linx

#endif
