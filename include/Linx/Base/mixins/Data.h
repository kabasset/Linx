// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_MIXINS_DATA_H
#define LINX_BASE_MIXINS_DATA_H

#include "Linx/Base/Functional.h"
#include "Linx/Base/Reduction.h"
#include "Linx/Base/mixins/Arithmetic.h"
#include "Linx/Base/mixins/Math.h"

#include <Kokkos_StdAlgorithms.hpp>
#include <string>
#include <utility> // integer_sequence, size_t

namespace Linx {

namespace Impl {

template <typename TView>
struct OffsetFiller {
  KOKKOS_INLINE_FUNCTION void operator()(auto... is) const
  {
    auto ptr = &m_view(is...);
    *ptr = ptr - m_ref;
  }
  TView m_view;
  const typename TView::value_type* m_ref; // FIXME const_pointer?
};

template <typename TFunc, typename TOut, typename TIns, std::size_t... Is>
class Generator {
public:

  KOKKOS_INLINE_FUNCTION Generator(TFunc func, const TOut& out, const TIns& ins) : m_func(func), m_out(out), m_ins(ins)
  {}

  KOKKOS_INLINE_FUNCTION void operator()(auto... is) const
  {
    m_out(is...) = m_func(get<Is>(m_ins)(is...)...);
  }

private:

  TFunc m_func;
  TOut m_out;
  TIns m_ins;
};

// /**
//  * @brief Index-based specialization without input arrays.
//  */
// template <typename TFunc, typename TOut, std::size_t... Is>
// class Generator<TFunc, TOut, void, Is...> {
// public:

//   KOKKOS_INLINE_FUNCTION Generator(TFunc, const TOut& out) : m_func(m_func), m_out(out) {}

//   KOKKOS_INLINE_FUNCTION void operator()(auto... is) const
//   {
//     m_out(is...) = m_func(is...);
//   }

// private:

//   TFunc m_func;
//   TOut m_out;
// };

} // namespace Impl

/**
 * @ingroup pixelwise
 * @brief Default `DataMixin` arithmetic tag.
 * 
 * Uses `BooleanArithmetic` if the element type is `bool`, otherwise uses `EuclidArithmetic`.
 */
template <typename T, typename TDerived>
using DataArithmeticMixin = std::conditional_t<
    std::is_same_v<std::remove_cvref_t<T>, bool>,
    BooleanArithmeticMixin<T, TDerived>,
    EuclidArithmeticMixin<T, TDerived>>;

/**
 * @ingroup pixelwise
 * @brief Data view mixin.
 * 
 * @tparam T The value type
 * @tparam TArithmetic The arithmetic tag
 * @tparam TDerived The derived class
 */
template <typename T, typename TArithmeticMixin, typename TDerived>
struct DataMixin : public TArithmeticMixin, public MathFunctionsMixin<T, TDerived> {
  using Arithmetic = TArithmeticMixin;

  /**
   * @brief Label.
   */
  decltype(auto) label() const
  {
    return LINX_CRTP_CONST_DERIVED.base().label();
  }

  /**
   * @brief Container size.
   */
  KOKKOS_INLINE_FUNCTION auto size() const
  {
    return LINX_CRTP_CONST_DERIVED.base().size();
  }

  /**
   * @brief Container size as a signed integer.
   */
  KOKKOS_INLINE_FUNCTION auto ssize() const
  {
    using Out = std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(size())>>;
    return static_cast<Out>(size());
  }

  /**
   * @brief Pointer to the raw data.
   */
  KOKKOS_INLINE_FUNCTION T* data() const
  {
    if constexpr (TDerived::n == 0) {
      return nullptr;
    } else {
      return LINX_CRTP_CONST_DERIVED.base().data();
    }
  }

  /**
   * @brief Pointer to the read-only raw data.
   */
  KOKKOS_INLINE_FUNCTION const T* cdata() const
  {
    return const_cast<const T*>(data());
  }

  /**
   * @brief Test whether the view is empty.
   */
  KOKKOS_INLINE_FUNCTION bool empty() const
  {
    return size() == 0;
  }

  /**
   * @brief Fill the view with a single value.
   */
  const TDerived& fill(const T& value) const
  {
    // TODO if is root() and value type is compatible (i.e. no string?)
    // TODO then use Kokkos::deep_copy()
    return generate("fill", Constant(value));
  }

  /**
   * @brief Fill the view with address offsets from the data to the elements.
   * 
   * This is a shortcut for `generate_offsets(data())`.
   */
  const TDerived& generate_offsets() const
  {
    return generate_offsets(data());
  }

  /**
   * @brief Fill the view with address offsets from some reference.
   * 
   * Conceptually, this function performs:
   * 
   * \code
   * for (auto p : view.domain()) {
   *   view.at(p) = &view.at(p) - ref;
   * }
   * \endcode
   */
  const TDerived& generate_offsets(const T* ref) const
  {
    const auto& derived = LINX_CRTP_CONST_DERIVED;
    using Space = typename TDerived::execution_space;
    for_each<Space>("generate_offsets()", derived.domain(), Impl::OffsetFiller<TDerived>(derived, ref));
    return derived;
  }

  /**
   * @brief Copy the values from another view.
   */
  const TDerived& copy_from(const auto& view) const
  {
    // TODO use Kokkos::deep_copy wherever possible
    return generate(compose_label("copy", view), Forward(), view);
  }

  /**
   * @brief Copy the values to another view.
   */
  const TDerived& copy_to(const auto& view) const
  {
    // TODO use Kokkos::deep_copy wherever possible
    return transform(compose_label("copy", *this), Forward(), view, *this);
  }

  /**
   * @brief Apply a function to each element.
   * 
   * @param label A label for debugging
   * @param func The function
   * @param inputs Optional input views
   * 
   * The first argument of the function is the element of the view itself.
   * If other images are passed as input, their elements are respectively passed to the function.
   * In this case, it is recommended to avoid side effects and to pass the inputs as readonly.
   * 
   * In other words:
   * 
   * \code
   * view.transform(label, func, a, b);
   * \endcode
   * 
   * conceptually performs:
   * 
   * \code
   * for (auto p : view.domain()) {
   *   view[p] = func(Linx::as_readonly(view)[p], Linx::as_readonly(a)[p], Linx::as_readonly(b)[p]);
   * }
   * \endcode
   * 
   * and is equivalent to:
   * 
   * \code
   * view.generate(label, func, view, a, b);
   * \endcode
   * 
   * @see `generate()`
   */
  const TDerived& transform(const std::string& label, auto&& func, const auto&... inputs) const
  {
    const auto& derived = as_readonly(LINX_CRTP_CONST_DERIVED);
    return LINX_CRTP_CONST_DERIVED
        .generate_with_side_effects(label, LINX_FORWARD(func), derived, try_as_readonly(inputs)...);
  }

  /**
   * @brief Assign each element according to a function.
   * 
   * @param label A label for debugging
   * @param func The function
   * @param inputs Optional input images
   * 
   * The arguments of the function are the elements of the input images, if any, i.e.:
   * 
   * \code
   * image.generate(label, func, a, b);
   * \endcode
   * 
   * conceptually performs:
   * 
   * \code
   * for (auto p : image.domain()) {
   *   image[p] = func(a[p], b[p]);
   * }
   * \endcode
   * 
   * @see `transform()`
   */
  const TDerived& generate(const std::string& label, auto&& func, const auto&... inputs) const
  {
    return generate_with_side_effects(label, LINX_FORWARD(func), try_as_readonly(inputs)...);
  }

  /**
   * @brief Assign each element according to a function.
   * 
   * @param label A label for debugging
   * @param func The function
   * @param others Optional views the function acts on
   * 
   * The arguments of the function are the elements of the views, if any, i.e.:
   * 
   * \code
   * view.generate_with_side_effects(label, func, a, b);
   * \endcode
   * 
   * conceptually performs:
   * 
   * \code
   * for (auto p : view.domain()) {
   *   view[p] = func(a[p], b[p]);
   * }
   * \endcode
   * 
   * The domain of the optional views must include the view domain.
   * 
   * The function is allowed to have side effects, i.e., to modify its arguments.
   * In this case, the elements of the optional views are effectively modified.
   * If the function has no side effect, it is preferrable to use `generate()` instead.
   * 
   * @see `DataMixin::transform()`
   * @see `DataMixin::generate()`
   */
  template <typename TFunc, typename... Ts>
  const TDerived& generate_with_side_effects(const std::string& label, TFunc&& func, const Ts&... others) const
  {
    generate_with_side_effects_impl(
        label,
        LINX_FORWARD(func),
        Tuple<Ts...>(others...),
        std::make_index_sequence<sizeof...(others)>());
    return LINX_CRTP_CONST_DERIVED;
  }

  template <typename TFunc, typename TIns, std::size_t... Is>
  void generate_with_side_effects_impl(
      const std::string& label,
      TFunc func,
      const TIns& others,
      std::index_sequence<Is...>) const // TODO private
  {
    using Generator = Impl::Generator<TFunc, TDerived, TIns, Is...>;
    using Space = typename TDerived::execution_space;
    for_each<Space>(
        label,
        LINX_CRTP_CONST_DERIVED.domain(),
        Generator(LINX_FORWARD(func), LINX_CRTP_CONST_DERIVED, others));
  }

  /**
   * @brief Test whether the view contains a given value.
   */
  bool contains(const T& value) const
  {
    const auto& derived = as_readonly(LINX_CRTP_CONST_DERIVED);
    return transform_reduce("contains()", Equal(value), Or(), derived);
  }

  /**
   * @brief Test whether the view contains NaNs.
   */
  bool contains_nan() const
  {
    const auto& derived = as_readonly(LINX_CRTP_CONST_DERIVED);
    return transform_reduce("contains_nan()", IsNan(), Or(), derived);
  }

  /**
   * @brief Test whether all elements are equal to a given value.
   * 
   * If the view is empty, return `false`.
   */
  bool contains_only(const T& value) const
  {
    const auto& derived = as_readonly(LINX_CRTP_CONST_DERIVED);
    return transform_reduce("contains_only()", Equal(value), And(), derived);
  }

  /**
   * @brief Equality of values only.
   * 
   * No check is performed on the domain,
   * and `rhs` does not even has to have a domain.
   * Typically, it can be a lambda function.
   * 
   * @see `operator==`
   */
  bool matches(const NotConvertibleTo<T> auto& rhs) const
  {
    return transform_reduce("matches()", Equal(), And(), LINX_CRTP_CONST_DERIVED, rhs);
  }

  /**
   * @brief Equality of size and values.
   */
  bool operator==(const auto& rhs) const
  {
    return (LINX_CRTP_CONST_DERIVED.size() == rhs.size()) && matches(rhs); // TODO .domain()?
  }

  /**
   * @brief Inequality.
   */
  bool operator!=(const auto& rhs) const
  {
    return not(*this == rhs);
  }
};

template <typename T, typename TArithmeticMixin, typename TDerived>
TDerived generate(
    const std::string& label,
    const auto& func,
    const DataMixin<T, TArithmeticMixin, TDerived>& in0,
    const auto&... ins)
{
  auto out = +in0;
  out.transform(label, func, ins...);
  return out;
}

} // namespace Linx

#endif