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
 * @brief Non-resizable 1D container with Euclid arithmetics and element-wise functions.
 * 
 * @tparam T The element value type
 * @tparam N The size, or -1 for runtime size
 */
template <typename T, int N, typename TContainer = SequenceContainer<T, N>>
class Sequence :
    public DataMixin<T, DataArithmeticMixin<T, Sequence<T, N, TContainer>>, Sequence<T, N, TContainer>>,
    public RangeMixin<true, T, Sequence<T, N, TContainer>> {
public:

  // FIXME most aliases and methods to DataMixin

  static constexpr int n = N; ///< The size parameter
  using Container = TContainer; ///< The underlying container type
  using Domain = Span<Index>;

  using memory_space = typename Container::memory_space;
  using execution_space = typename Container::execution_space;

  using value_type = typename Container::value_type; ///< The raw element value type
  using element_type = std::decay_t<value_type>; ///< The decayed element value type
  using size_type = typename Container::size_type; ///< The index and size type
  using difference_type = std::ptrdiff_t; ///< The index difference type
  using reference = typename Container::reference_type; ///< The element reference type
  using pointer = typename Container::pointer_type; ///< The element pointer type
  using iterator = decltype(Kokkos::Experimental::begin(Container())); ///< The iterator type
  using const_iterator = decltype(Kokkos::Experimental::cbegin(Container())); ///< The constant iterator type
  /**
   * @brief Constructor.
   * 
   * @param label The sequence label
   * @param size The sequence size
   * @param values, value The sequence values
   * @param begin, end Pointer or iterator to the values beginning and end
   * @param container A compatible container
   * @param args Arguments forwarded to the container
   * @param data Some external data to be viewed as a sequence
   * 
   * @warning If the size is set at compile time, the size parameter or value count must match it.
   */
  Sequence() : Sequence("") {}

  /**
   * @copydoc Sequence()
   */
  explicit Sequence(const std::string& label) : Sequence(label, std::max(0, n)) {}

  /**
   * @copydoc Sequence()
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
   * @copydoc Sequence()
   */
  explicit Sequence(std::integral auto size) : Sequence("", size) {}

  /**
   * @copydoc Sequence()
   */
  KOKKOS_INLINE_FUNCTION explicit Sequence(const Container& container) : m_container(container) {}

  /**
   * @copydoc Sequence()
   */
  KOKKOS_INLINE_FUNCTION explicit Sequence(Container&& container) : m_container(LINX_MOVE(container)) {}

  // /**
  //  * @copydoc Sequence()
  //  */
  // Sequence(T (&&values)[N]) : Sequence("", values, values + N) {}
  // FIXME incompatible with N = -1 => Specialize whole class?
  // Would be nice to enable type deduction, including for aliases like Position, e.g.
  // Position p({1, 2, 3}) -> Position<int, 3>

  /**
   * @copydoc Sequence()
   */
  Sequence(std::initializer_list<value_type> values) : Sequence("", values.begin(), values.end()) {}

  /**
   * @copydoc Sequence()
   */
  explicit Sequence(const std::string& label, std::initializer_list<value_type> values) :
      Sequence(label, values.begin(), values.end())
  {}

  /**
   * @copydoc Sequence()
   */
  explicit Sequence(const std::string& label, const std::ranges::range auto& values) :
      Sequence(label, std::ranges::begin(values), std::ranges::end(values))
  {}

  /**
   * @copydoc Sequence()
   */
  KOKKOS_INLINE_FUNCTION explicit Sequence(Forward, auto&&... args) : m_container(LINX_FORWARD(args)...) {}

  /**
   * @copydoc Sequence()
   */
  explicit Sequence(const std::string& label, std::input_iterator auto begin, std::input_iterator auto end) :
      Sequence(label, std::ranges::distance(begin, end))
  {
    this->assign(LINX_MOVE(begin));
  }

  /**
   * @copydoc Sequence()
   */
  explicit Sequence(const std::string& label, const value_type* begin, const value_type* end) :
      Sequence(label, end - begin)
  {
    this->assign(begin);
  }

  /**
   * @copydoc Sequence()
   */
  KOKKOS_INLINE_FUNCTION explicit Sequence(Wrap<value_type*> data, std::integral auto size) :
      m_container(data.value, size)
  {}

  /**
   * @brief Sequence().
   */
  template <typename U>
  [[deprecated]] Sequence(const std::string& label, Constant<U> value, int size = std::abs(n)) : Sequence(label, size)
  {
    this->fill(value.value);
  }

  /**
   * @brief Sequence().
   */
  template <typename U>
  [[deprecated]] Sequence(Constant<U> value, int size = std::abs(n)) : Sequence("", value, size)
  {}

  /**
   * @brief The rank is always 1.
   */
  static constexpr auto rank()
  {
    return 1;
  }

  /**
   * @brief Container span.
   */
  KOKKOS_INLINE_FUNCTION Domain domain() const
  {
    return Domain(0, m_container.size());
  }

  /**
   * @brief Container size, for compatibility with `DataContainer`.
   */
  Sequence<size_type, 1> shape() const
  {
    return Sequence<size_type, 1>("shape", m_container.size());
  }

  /**
   * @brief The extent along the first axis (the size), for compatibility.
   */
  KOKKOS_INLINE_FUNCTION void extent(std::integral auto = 0)
  {
    return this->size();
  }

  /**
   * @brief The stride is always 1.
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
   * @brief Access the i-th element.
   */
  KOKKOS_INLINE_FUNCTION reference operator[](std::integral auto i) const
  {
    return m_container(i);
  }

  /**
   * @brief Access the i-th element, for compatibility with `DataContainer`.
   */
  KOKKOS_INLINE_FUNCTION reference operator()(std::integral auto i) const
  {
    return m_container(i);
  }

  /**
   * @brief Iterator to the beginning.
   */
  KOKKOS_INLINE_FUNCTION iterator begin() const
  {
    return Kokkos::Experimental::begin(m_container);
  }

  /**
   * @brief Iterator to the end.
   */
  KOKKOS_INLINE_FUNCTION iterator end() const
  {
    return Kokkos::Experimental::end(m_container);
  }

  /**
   * @brief Constant iterator to the beginning.
   */
  KOKKOS_INLINE_FUNCTION const_iterator cbegin() const
  {
    return Kokkos::Experimental::cbegin(m_container);
  }

  /**
   * @brief Constant iterator to the end.
   */
  KOKKOS_INLINE_FUNCTION const_iterator cend() const
  {
    return Kokkos::Experimental::cend(m_container);
  }

  /**
   * @brief Stream insertion.
   */
  friend std::ostream& operator<<(std::ostream& os, const Sequence& sequence)
  {
    auto hosted = on_host(sequence);
    os << "[" << hosted[0];
    for (std::size_t i = 1; i < hosted.size(); ++i) {
      os << ", " << hosted[i];
    }
    os << "]";
    return os;
  }

private:

  /**
   * @brief The Kokkos container.
   */
  Container m_container;
};

template <typename T, int N>
using GPosition = Sequence<T, N, SequenceContainer<T, N, Kokkos::HostSpace>>;

template <int N>
using Position = GPosition<Index, N>;

template <typename T, int N, typename TContainer = SequenceContainer<T, N>>
Sequence(T (&&)[N]) -> Sequence<T, N, TContainer>;

template <typename T, int N, typename TContainer = SequenceContainer<T, N>>
Sequence(const std::string&, T (&&)[N]) -> Sequence<T, N, TContainer>;

template <typename T, int N, typename TContainer = SequenceContainer<T, N>>
Sequence(const char*, T (&&)[N]) -> Sequence<T, N, TContainer>;

/**
 * @brief Get the i-th element of an array, or some fallback value if out of bounds.
 */
template <int I, typename T, int N, typename TContainer, typename U>
KOKKOS_INLINE_FUNCTION U get_or(const Sequence<T, N, TContainer>& in, U fallback) // FIXME ArrayLike?
{
  if constexpr (N == -1) {
    return (I < std::size(in)) ? static_cast<U>(in[I]) : fallback;
  } else {
    return I < N ? static_cast<U>(in[I]) : fallback;
  }
}

/**
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
 * @brief Perform a shallow copy of a sequence, as an atomic sequence.
 */
template <typename T, int N, typename TContainer>
KOKKOS_INLINE_FUNCTION decltype(auto) as_atomic(const Sequence<T, N, TContainer>& in)
{
  using Out = Sequence<T, N, typename Rebind<TContainer>::AsAtomic>;
  return Out(Linx::Forward {}, in.container());
}

/**
 * @brief Copy the data to host if on device.
 */
template <typename T, int N, typename TContainer>
decltype(auto) on_host(const Sequence<T, N, TContainer>& in)
{
  return on_device<Kokkos::HostSpace>(in);
}

/**
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
template <ArrayLike TIn, ArrayLike TOut>
void copy_to(const TIn& in, const TOut& out) // FIXME replace with/update DataMixin::copy_from/to
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

/**
 * @ingroup creation
 * @brief Static-size sequence of evenly spaced values.
 */
template <int N, typename TSpace = Kokkos::DefaultExecutionSpace, typename T>
auto range(const std::string& label, T start = Limits<T>::zero(), T step = Limits<T>::one())
{
  return Linx::Sequence<T, N, SequenceContainer<T, N, TSpace>>(label).range(start, step);
}

/**
 * @ingroup creation
 * @brief Dynamic-size sequence of evenly spaced values.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T>
auto range(std::integral auto size, const std::string& label, T start = Limits<T>::zero(), T step = Limits<T>::one())
{
  return Linx::Sequence<T, -1, SequenceContainer<T, -1, TSpace>>(label, size).range(start, step);
}

/**
 * @ingroup creation
 * @brief Static-size sequence of evenly spaced values between bounds.
 */
template <int N, typename TSpace = Kokkos::DefaultExecutionSpace, typename T>
auto linspace(const std::string& label, const Span<T>& bounds)
{
  return Linx::Sequence<T, N, SequenceContainer<T, N, TSpace>>(label).linspace(bounds); // TODO uninitialized
}

/**
 * @ingroup creation
 * @brief Dynamic-size sequence of evenly spaced values between bounds.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T>
auto linspace(std::integral auto size, const std::string& label, const Span<T>& bounds)
{
  return Linx::Sequence<T, -1, SequenceContainer<T, -1, TSpace>>(label, size).linspace(bounds); // TODO uninitialized
}

/**
 * @ingroup creation
 * @brief Generate a static-size sequence.
 * @tparam N The size
 * @param label The label
 * @param func The generator
 */
template <int N, typename TSpace = Kokkos::DefaultExecutionSpace>
auto generate(const std::string& label, const auto& func)
{
  static_assert(N >= 0);
  using T = std::remove_cvref_t<decltype(func(0))>;
  auto out = Sequence<T, N, SequenceContainer<T, N, TSpace>>(label); // TODO uninitialized
  // for_each<TSpace>("generate", KOKKOS_LAMBDA(int i) { out[i] = func(i); }); // FIXME
  return out;
}

/**
 * @ingroup creation
 * @brief Generate a dynamic-size sequence.
 * @param label The label
 * @param func The generator
 * @param size The size
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
auto generate(std::integral auto size, const std::string& label, const auto& func)
{
  using T = std::remove_cvref_t<decltype(func(0))>;
  auto out = Sequence<T, -1, SequenceContainer<T, -1, TSpace>>(label, size); // TODO uninitialized
  // for_each<TSpace>("generate", KOKKOS_LAMBDA(int i) { out[i] = func(i); }); // FIXME
  return out;
}

/**
 * @ingroup creation
 * @brief Copy an array as a static-size sequence.
 * 
 * If the input array is larger than `N`, only the `N` first values are copied.
 * If it is smaller, the remaining values are default-initialized.
 */
template <int N, typename TSpace = Kokkos::DefaultExecutionSpace>
auto resize(const std::string& label, const ArrayLike auto& in)
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
 * 
 * If the input array is larger than `size`, only the `size` first values are copied.
 * If it is smaller, the remaining values are default-initialized.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace>
auto resize(std::integral auto size, const std::string& label, const ArrayLike auto& in)
{
  using T = std::remove_cvref_t<decltype(in[0])>;
  Sequence<T, -1, SequenceContainer<T, -1, TSpace>> out(label, size);
  copy_to(in, out);
  return out;
}

/**
 * @ingroup creation
 * @brief Copy a list as a static-size sequence.
 * 
 * If the input list is larger than `N`, only the `N` first values are copied.
 * If it is smaller, the remaining values are default-initialized.
 */
template <int N, typename TSpace = Kokkos::DefaultExecutionSpace, typename T>
auto resize(const std::string& label, std::initializer_list<T> in)
{
  return resize<N, TSpace>(label, Sequence<T, N, SequenceContainer<T, N, TSpace>>(in));
}

/**
 * @ingroup creation
 * @brief Copy a list as a dynamic-size sequence.
 * 
 * If the input list is larger than `size`, only the `size` first values are copied.
 * If it is smaller, the remaining values are default-initialized.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename T>
auto resize(std::integral auto size, const std::string& label, std::initializer_list<T> in)
{
  return resize<TSpace>(size, label, Sequence<T, -1, SequenceContainer<T, -1, TSpace>>(in));
}

template <int M, typename T, int N>
auto pad(const GPosition<T, N>& in) // FIXME merge with resize
{
  using U = std::decay_t<T>;
  GPosition<U, M> out(compose_label("pad", in));
  copy_to(in, out);
  return out;
}

template <int M, typename T, int N>
auto pad(const GPosition<T, N>& in, const T& value)
{
  using U = std::decay_t<T>;
  GPosition<U, M> out(compose_label("pad", in, value), Constant(value));
  copy_to(in, out);
  return out;
}

} // namespace Linx

#endif
