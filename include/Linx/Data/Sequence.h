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

/**
 * @ingroup arrays
 * @brief Non-resizable 1D array.
 * 
 * @tparam T The element value type
 * @tparam N The size, or -1 for dynamic size
 * @tparam TContainer The underlying container type
 * 
 * By default, sequence elements are default-initialized.
 * Copy constructor and copy assignment operator perform shallow copy.
 * 
 * @see arrays
 * @see `DataMixin`
 * @see `RangeMixin`
 */
template <typename T, int N, typename TContainer = SequenceContainer<T, N>>
class Sequence :
    public DataMixin<T, DataArithmeticMixin<T, Sequence<T, N, TContainer>>, Sequence<T, N, TContainer>>,
    public RangeMixin<true, T, Sequence<T, N, TContainer>> {
public:

  // TODO most aliases and methods to DataMixin

  static constexpr int n = N; ///< The size parameter
  using Container = TContainer; ///< The underlying container type
  using Domain = Slice<Index>; ///< The domain type

  using memory_space = typename Container::memory_space; ///< The memory space
  using execution_space = typename Container::execution_space; ///< The default execution space

  using value_type = typename Container::value_type; ///< The possibly const-qualified element type
  using element_type = std::remove_cvref_t<value_type>; ///< The element type
  using size_type = typename Container::size_type; ///< The index and size type
  using difference_type = std::ptrdiff_t; ///< The index difference type
  using reference = typename Container::reference_type; ///< The element reference type
  using pointer = typename Container::pointer_type; ///< The element pointer type
  using iterator = decltype(Kokkos::Experimental::begin(Container())); ///< The iterator type
  using const_iterator = decltype(Kokkos::Experimental::cbegin(Container())); ///< The constant iterator type

  /**
   * @brief Default constructor.
   */
  Sequence() : Sequence("") {}

  /**
   * @brief Constructor.
   * @param label The sequence label
   */
  explicit Sequence(const std::string& label) : Sequence(label, std::max(0, n)) {}

  /**
   * @brief Constructor.
   * @param label The sequence label
   * @param size The sequence size
   * 
   * @warning If the size is static (`n != -1`), the `size` parameter must match it.
   */
  explicit Sequence(const std::string& label, std::integral auto size) : m_container(label)
  {
    if constexpr (n < 1) {
      Kokkos::resize(m_container, size);
    } else {
      // FIXME assert(size == n)
    }
  }

  /**
   * @brief Constructor.
   * @param size The sequence size
   * 
   * @warning If the size is static (`n != -1`), the `size` parameter must match it.
   */
  explicit Sequence(std::integral auto size) : Sequence("", size) {}

  /**
   * @copydoc Sequence()
   */
  [[deprecated]] KOKKOS_INLINE_FUNCTION explicit Sequence(const Container& container) : m_container(container) {}

  /**
   * @copydoc Sequence()
   */
  [[deprecated]] KOKKOS_INLINE_FUNCTION explicit Sequence(Container&& container) : m_container(LINX_MOVE(container)) {}

  /**
   * @brief Constructor.
   * @param values The sequence values
   * 
   * This constructor is not explicit, such that brace-enclosed list may be implicitely converted to sequences.
   * 
   * @warning If the size is static (`n != -1`), the value count must match it.
   */
  Sequence(std::initializer_list<value_type> values) : Sequence("", values.begin(), values.end()) {}

  /**
   * @brief Constructor.
   * @param label The sequence label
   * @param values The sequence values
   * 
   * @warning If the size is static (`n != -1`), the value count must match it.
   */
  explicit Sequence(const std::string& label, std::initializer_list<value_type> values) :
      Sequence(label, values.begin(), values.end())
  {}

  /**
   * @brief Constructor.
   * @param label The sequence label
   * @param values The sequence values
   * 
   * @warning If the size is static (`n != -1`), the value count must match it.
   */
  explicit Sequence(const std::string& label, const std::ranges::range auto& values) :
      Sequence(label, std::ranges::begin(values), std::ranges::end(values))
  {}

  /**
   * @brief Forwarding constructor.
   * @param args The parameters forwarded to the container's constructor
   */
  KOKKOS_INLINE_FUNCTION explicit Sequence(Forward, auto&&... args) : m_container(LINX_FORWARD(args)...) {}

  /**
   * @brief Constructor.
   * @param label The sequence label
   * @param begin, end The sequence values iterators
   * 
   * @warning If the size is static (`n != -1`), the value count must match it.
   */
  explicit Sequence(const std::string& label, std::input_iterator auto begin, std::input_iterator auto end) :
      Sequence(label, std::ranges::distance(begin, end))
  {
    this->assign(LINX_MOVE(begin));
  }

  /**
   * @brief Constructor.
   * @param label The sequence label
   * @param begin, end The sequence values pointers
   * 
   * @warning If the size is static (`n != -1`), the value count must match it.
   */
  explicit Sequence(const std::string& label, const value_type* begin, const value_type* end) :
      Sequence(label, end - begin)
  {
    this->assign(begin);
  }

  /**
   * @brief Wrapping constructor.
   * @param data The wrapped data
   * @param size The sequence size
   * 
   * The resulting sequence does not own the data.
   * It won't manage its memory or ensure it is valid.
   * 
   * @warning If the size is static (`n != -1`), the value count must match it.
   */
  KOKKOS_INLINE_FUNCTION explicit Sequence(Wrap<value_type*> data, std::integral auto size) :
      m_container(data.value, size)
  {}

  /**
   * @brief Constant constructor.
   */
  template <typename U>
  [[deprecated]] Sequence(const std::string& label, Constant<U> value, int size = std::abs(n)) : Sequence(label, size)
  {
    this->fill(value.value);
  }

  /**
   * @brief Constant constructor.
   */
  template <typename U>
  [[deprecated]] Sequence(Constant<U> value, int size = std::abs(n)) : Sequence("", value, size)
  {}

  /**
   * @brief Array rank: 1.
   */
  static constexpr auto rank()
  {
    return 1;
  }

  /**
   * @brief Array domain: `Slice(0, size())`.
   */
  KOKKOS_INLINE_FUNCTION Domain domain() const
  {
    return Domain(0, m_container.size());
  }

  /**
   * @brief Array shape: `{size()}`.
   */
  Sequence<size_type, 1> shape() const
  {
    return Sequence<size_type, 1>("shape", m_container.size());
  }

  /**
   * @brief Extent along the first (and only) axis: `size()`.
   */
  KOKKOS_INLINE_FUNCTION void extent(std::integral auto = 0)
  {
    return this->size();
  }

  /**
   * @brief Stride along the first (and only) axis: 1.
   */
  static constexpr auto stride(std::integral auto = 0)
  {
    return 1;
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
    return m_container(0);
  }

  /**
   * @brief Access the last element.
   */
  KOKKOS_INLINE_FUNCTION reference back() const
  {
    return m_container(m_container.size() - 1);
  }

  /**
   * @brief Access the i-th element.
   */
  KOKKOS_INLINE_FUNCTION reference operator()(std::integral auto i) const
  {
    return m_container(i);
  }

  /**
   * @brief Access the i-th element.
   */
  KOKKOS_INLINE_FUNCTION reference operator[](std::integral auto i) const
  {
    return m_container(i);
  }

  /**
   * @brief Iterator to the beginning.
   */
  KOKKOS_INLINE_FUNCTION iterator begin() const // FIXME in RangeMixin already
  {
    return Kokkos::Experimental::begin(m_container);
  }

  /**
   * @brief Iterator to the end.
   */
  KOKKOS_INLINE_FUNCTION iterator end() const // FIXME in RangeMixin already
  {
    return Kokkos::Experimental::end(m_container);
  }

  /**
   * @brief Constant iterator to the beginning.
   */
  KOKKOS_INLINE_FUNCTION const_iterator cbegin() const // FIXME in RangeMixin already
  {
    return Kokkos::Experimental::cbegin(m_container);
  }

  /**
   * @brief Constant iterator to the end.
   */
  KOKKOS_INLINE_FUNCTION const_iterator cend() const // FIXME in RangeMixin already
  {
    return Kokkos::Experimental::cend(m_container);
  }

  /**
   * @brief Stream insertion.
   * 
   * @warning This may involve a copy on host.
   */
  friend std::ostream& operator<<(std::ostream& os, const Sequence& sequence)
  {
    const auto& sequence_on_host = on_host(sequence);
    os << "[" << sequence_on_host[0];
    for (std::size_t i = 1; i < sequence_on_host.size(); ++i) {
      os << ", " << sequence_on_host[i];
    }
    os << "]";
    return os;
  }

private:

  Container m_container; ///< The Kokkos container.
};

/**
 * @brief Sequence on host.
 */
template <typename T, int N>
using GPosition = Sequence<T, N, SequenceContainer<T, N, Kokkos::HostSpace>>;

/**
 * @brief Integral-valued sequence on host.
 */
template <int N>
using Position = GPosition<Index, N>;

template <typename T, int N, typename TContainer = SequenceContainer<T, N>>
Sequence(T (&&)[N]) -> Sequence<T, N, TContainer>;

template <typename T, int N, typename TContainer = SequenceContainer<T, N>>
Sequence(const std::string&, T (&&)[N]) -> Sequence<T, N, TContainer>;

template <typename T, int N, typename TContainer = SequenceContainer<T, N>>
Sequence(const char*, T (&&)[N]) -> Sequence<T, N, TContainer>;

/**
 * @brief I-th element of an array, or some fallback value if out of bounds.
 */
template <int I, typename T>
KOKKOS_INLINE_FUNCTION T get_or(const LegacyArray auto& in, T fallback)
{
  return (I < std::size(in)) ? static_cast<T>(in[I]) : fallback;
}

/**
 * @relatesalso Sequence
 * @brief Perform a shallow copy of a sequence, as a readonly sequence.
 * 
 * If the input sequence is aleady readonly, then this is a no-op.
 */
template <typename T, int N, typename TContainer>
KOKKOS_INLINE_FUNCTION decltype(auto) as_readonly(const Sequence<T, N, TContainer>& in)
{
  if constexpr (std::is_const_v<T>) {
    return in;
  } else {
    using Out = Sequence<const T, N, typename Rebind<TContainer>::AsReadonly>;
    return Out(Linx::Forward {}, in.container());
  }
}

/**
 * @relatesalso Sequence
 * @brief Perform a shallow copy of a sequence, as an atomic sequence.
 */
template <typename T, int N, typename TContainer>
KOKKOS_INLINE_FUNCTION decltype(auto) as_atomic(const Sequence<T, N, TContainer>& in)
{
  using Out = Sequence<T, N, typename Rebind<TContainer>::AsAtomic>;
  return Out(Linx::Forward {}, in.container());
}

/**
 * @relatesalso Sequence
 * @brief Copy the data to host if on device.
 */
template <typename T, int N, typename TContainer>
decltype(auto) on_host(const Sequence<T, N, TContainer>& in)
{
  return on_device<Kokkos::HostSpace>(in);
}

/**
 * @relatesalso Sequence
 * @brief Copy the data to a given memory space if not already accessible from it.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace::memory_space, typename T, int N, typename TContainer>
decltype(auto) on_device(const Sequence<T, N, TContainer>& in)
{
  if constexpr (Kokkos::SpaceAccessibility<TSpace, typename TContainer::memory_space>::accessible) {
    return in;
  } else {
    auto container = Kokkos::create_mirror_view_and_copy(TSpace(), in.container());
    return Sequence<T, N, decltype(container)>(Forward {}, LINX_MOVE(container));
  }
}

/**
 * @brief Copy as many elements as possible from `in` to `out`.
 */
template <LegacyArray TIn, LegacyArray TOut>
void copy_to(const TIn& in, const TOut& out) // FIXME rename as copy_intersection or even rm
{
  auto domain = Slice(0, std::min<int>(std::size(in), std::size(out)));
  for_each<typename TOut::execution_space>("copy_to()", domain, KOKKOS_LAMBDA(int i) { out[i] = in[i]; });
}

/**
 * @ingroup creation
 * @brief Default-initialized sequence with the same memory layout (space, size, stride) as an input sequence.
 */
template <typename U = void, typename T, int N, typename TContainer>
auto same_layout(const std::string& label, const Sequence<T, N, TContainer>& in)
{
  return Sequence<typename Rebind<T>::As<U>, N, typename Rebind<TContainer>::As<U>>(
      Forward(),
      same_layout<U>(label, in.container()));
}

/**
 * @ingroup creation
 * @brief Static-size sequence filled with a single value.
 */
template <int N, typename TSpace = Kokkos::DefaultExecutionSpace>
auto fill(const std::string& label, const auto& value)
{
  using T = std::remove_cvref_t<decltype(value)>;
  auto out = Sequence<T, N, SequenceContainer<T, N, TSpace>>(label); // FIXME uninitialized
  if (value != T {}) {
    out.fill(value);
  }
  return out;
}

/**
 * @ingroup creation
 * @brief Dynamic-size sequence filled with a single value.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
auto fill(std::integral auto size, const std::string& label, const auto& value)
{
  using T = std::remove_cvref_t<decltype(value)>;
  auto out = Sequence<T, -1, SequenceContainer<T, -1, TSpace>>(label, size); // FIXME uninitialized
  if (value != T {}) {
    out.fill(value);
  }
  return out;
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
