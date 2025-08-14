// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_REDUCTION_H
#define LINX_BASE_REDUCTION_H

#include "Linx/Base/Containers.h"
#include "Linx/Base/Functional.h"
#include "Linx/Base/Packs.h"
#include "Linx/Base/Slice.h" // OutOfBounds
#include "Linx/Base/Types.h"
#include "Linx/Base/mixins/Data.h"

#include <Kokkos_Core.hpp>
#include <Kokkos_StdAlgorithms.hpp>
#include <string>
#include <utility> // integer_sequence, size_t

namespace Linx {

namespace Impl {

/**
 * @brief Functor which return a value for each position, typically using one or several images.
 */
template <typename T, typename TFunc, typename TIns, std::size_t... Is>
class Projection {
public:

  using value_type = std::remove_cv_t<T>; ///< The projection value type

  /**
   * @brief Constructor.
   */
  KOKKOS_INLINE_FUNCTION Projection(const TFunc& func, const TIns& ins) : m_func(func), m_ins(ins) {}

  /**
   * @brief Call `func(get<0>(ins)(position...), get<1>(ins)(position...), ...)`.
   */
  KOKKOS_INLINE_FUNCTION value_type operator()(auto... position) const
  {
    return m_func(get<Is>(m_ins)(position...)...);
  }

private:

  TFunc m_func; ///< The multivariate function
  TIns m_ins; ///< The input tuple
};

/**
 * @brief Kokkos-compliant reducer built from a binary operation functor.
 */
template <typename T, typename TFunc, typename TSpace>
class Reducer {
public:

  using reducer = Reducer; ///< This class, required for Kokkos' "concept"
  using value_type = std::remove_cv_t<T>; ///< The reduced value type
  using result_view_type = Kokkos::View<value_type, TSpace>; ///< The scalar result type, as a rank-0 view

  /**
   * @brief Value-based constructor.
   */
  KOKKOS_INLINE_FUNCTION Reducer(value_type& value, const TFunc& func, const T& identity) :
      m_view(&value),
      m_func(func),
      m_identity(identity)
  {}

  /**
   * @brief View-based constructor.
   */
  KOKKOS_INLINE_FUNCTION Reducer(const result_view_type& view, const TFunc& func, const T& identity) :
      m_view(view),
      m_func(func),
      m_identity(identity)
  {}

  /**
   * @brief Add a value.
   */
  KOKKOS_INLINE_FUNCTION void join(value_type& dst, const value_type& src) const
  {
    dst = m_func(dst, src);
  }

  /**
   * @brief Initialize a value to the identity element.
   */
  KOKKOS_INLINE_FUNCTION void init(value_type& value) const
  {
    value = m_identity;
  }

  /**
   * @brief Reference to the current value.
   */
  KOKKOS_INLINE_FUNCTION value_type& reference() const
  {
    return m_view();
  }

  /**
   * @brief View of the current value.
   */
  KOKKOS_INLINE_FUNCTION result_view_type view() const
  {
    return m_view;
  }

private:

  result_view_type m_view; ///< The view of the current value
  TFunc m_func; ///< The binary function
  value_type m_identity; ///< The identity element
};

/**
 * @brief Functor which combines a multivariate projection and a reducer.
 */
template <typename T, typename TProj, typename TRed, std::size_t... Is>
class ProjectionReducer {
public:

  static constexpr std::size_t n = sizeof...(Is); ///< The rank
  using value_type = std::remove_cv_t<T>; ///< The reduced value type

  /**
   * @brief Constructor.
   */
  KOKKOS_INLINE_FUNCTION ProjectionReducer(const TProj& projection, const TRed& reducer) :
      m_projection(projection),
      m_reducer(reducer)
  {}

  /**
   * @brief Call `reducer.join(tmp, projection(is...))`
   * @param args `is..., tmp`
   */
  template <typename... Ts>
  KOKKOS_INLINE_FUNCTION void operator()(Ts&&... args) const
  {
    auto tuple = forward_as_tuple(args...);
    static_assert(sizeof...(args) == n + 1);
    m_reducer.join(get<n>(tuple), m_projection(get<Is>(tuple)...));
  }

private:

  TProj m_projection; ///< The projection
  TRed m_reducer; ///< The reducer
};

/**
 * @brief Helper function to iterate over the pack parameters.
 */
template <typename TSpace, typename TRegion, typename TProj, typename TRed, std::size_t... Is>
void kokkos_reduce_impl(
    const std::string& label,
    const TRegion& region,
    const TProj& projection,
    const TRed& reducer,
    std::index_sequence<Is...>)
{
  if constexpr (TRegion::n == 0) {
    return;
  } else {
    using T = typename TRed::value_type;
    using ProjectionReducer = Impl::ProjectionReducer<T, TProj, TRed, Is...>;
    Kokkos::parallel_reduce(
        label,
        kokkos_execution_policy<TSpace>(region),
        ProjectionReducer(projection, reducer),
        reducer);
  }
}

} // namespace Impl

/**
 * @brief Apply a reduction over a region.
 * 
 * @param label Some label for debugging
 * @param region The region
 * @param projection The projection function
 * @param reducer The reduction function
 * 
 * The projection function takes as input a list of indices and outputs some value.
 * Images are projections.
 * 
 * The reducer satisfies Kokkos' `ReducerConcept`.
 * The `join()` method of the reducer is used for both intra- and inter-thread reduction.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace, typename TRegion, typename TProj, typename TRed>
void kokkos_reduce(const std::string& label, const TRegion& region, const TProj& projection, const TRed& reducer)
{
#define LINX_CASE_RANK(n) \
  case n: \
    if constexpr (is_nary<TProj, int, n>()) { \
      return Impl::kokkos_reduce_impl<TSpace>( \
          label, \
          pad<n>(region), \
          projection, \
          reducer, \
          std::make_index_sequence<n>()); \
    } else { \
      return; \
    }

  if constexpr (TRegion::n == -1) {
    switch (region.rank()) {
      case 0:
        return;
        LINX_CASE_RANK(1)
        LINX_CASE_RANK(2)
        LINX_CASE_RANK(3)
        LINX_CASE_RANK(4)
        LINX_CASE_RANK(5)
        LINX_CASE_RANK(6)
      default:
        throw Linx::OutOfBounds("Reduction dynamic rank", region.rank(), Segment<int>(0, 6));
    }
  } else {
    Impl::kokkos_reduce_impl<TSpace>(label, region, projection, reducer, std::make_index_sequence<TRegion::n>());
  }

#undef LINX_CASE_RANK
}

/**
 * @ingroup reduction
 * @brief Compute a reduction.
 * 
 * @param label A label for debugging
 * @param monoid The reduction monoid
 * @param in The input data container
 * 
 * The monoid is an associative binary operator functor, for which `identity_element()` is defined,
 * i.e. the following is available: `identity_element<T>(monoid)`, where `T` is the element type of `in`.
 */
template <typename TMonoid, typename TIn>
auto reduce(const std::string& label, const TMonoid& monoid, const TIn& in)
{
  using T = typename TIn::element_type;
  using Reducer = Impl::Reducer<T, TMonoid, Kokkos::HostSpace>;
  T value = identity_element<T>(monoid);
  kokkos_reduce<typename TIn::execution_space>(
      label,
      in.domain(),
      try_as_readonly(in),
      Reducer(value, monoid, identity_element<T>(monoid)));
  return value;
}

/**
 * @ingroup reduction
 * @brief Compute a reduction with mapping.
 * 
 * @param label A label for debugging
 * @param map The mapping functor
 * @param monoid The reduction monoid
 * @param ins Input data containers
 * 
 * For each position of the input domain, the elements of each input data container are passed to the mapping function
 * before the reduction monoid is applied, i.e., `transform_reduce("", map, monoid, a, b, c)` produces:
 * 
 * \code
 * map(a[p0], b[p0], c[p0]) + map(a[p1], b[p1], c[p1]) + ... + map(a[pN], b[pN], c[pN])
 * \endcode
 * 
 * where `p0, p1, ... , pN` are the positions in the image domain and `+` denotes the monoid operator.
 * 
 * Typically, the dot product of two containers `a` and `b` can be implemented as:
 * 
 * \code
 * transform_reduce("dot", Multiply(), Add(), a, b);
 * \endcode
 * 
 * @see `reduce()`
 */
template <typename TMap, typename TMonoid, typename... TIns>
auto transform_reduce(const std::string& label, const TMap& map, const TMonoid& monoid, const TIns&... ins)
{
  return transform_reduce_with_side_effects(label, map, monoid, try_as_readonly(ins)...);
}

namespace Impl {

/**
 * @brief Helper function to iterate over the pack.
 */
template <typename T, typename TMap, typename TMonoid, typename TIns, std::size_t... Is>
auto transform_reduce_with_side_effects_impl(
    const std::string& label,
    const TMap& map,
    const TMonoid& monoid,
    const TIns& ins,
    std::index_sequence<Is...>)
{
  const auto& in0 = get<0>(ins);
  using Space = LINX_DECLTYPE(in0)::execution_space; // FIXME test accessibility of all Is
  using Projection = Impl::Projection<T, TMap, TIns, Is...>;
  using Reducer = Impl::Reducer<T, TMonoid, Kokkos::HostSpace>;
  auto value = identity_element<T>(monoid);
  kokkos_reduce<Space>(label, in0.domain(), Projection(map, ins), Reducer(value, monoid, value));
  return value;
}

} // namespace Impl

/**
 * @copydoc transform_reduce()
 */
template <typename TMap, typename TMonoid, typename... TIns>
auto transform_reduce_with_side_effects(
    const std::string& label,
    const TMap& map,
    const TMonoid& monoid,
    const TIns&... ins)
{
  using T = LINX_DECLTYPE(map(at_origin(ins)...));
  return Impl::transform_reduce_with_side_effects_impl<T>(
      label,
      map,
      monoid,
      Tuple<TIns...>(ins...),
      std::make_index_sequence<sizeof...(TIns)>());
}

/**
 * @ingroup reduction
 * @brief Minimun value of a data container.
 */
template <typename TIn>
typename TIn::element_type min(const TIn& in)
{
  return reduce("min", Min(), in);
}

/**
 * @ingroup reduction
 * @brief Maximum value of a data container.
 */
template <typename TIn>
typename TIn::element_type max(const TIn& in)
{
  return reduce("max", Max(), in);
}

/**
 * @ingroup reduction
 * @brief Compute the sum of all elements of a data container.
 */
template <typename TIn>
typename TIn::element_type sum(const TIn& in) // TODO limit to DataMixins
{
  return reduce("sum", Add(), in);
}

/**
 * @ingroup reduction
 * @brief Compute the product of all elements of a data container.
 */
template <typename TIn>
typename TIn::element_type product(const TIn& in) // TODO limit to DataMixins
{
  return reduce("product", Multiply(), in);
}

/**
 * @ingroup reduction
 * @brief Compute the dot product of two data containers.
 */
template <typename TLhs, typename TRhs>
typename TLhs::element_type dot(const TLhs& lhs, const TRhs& rhs)
{
  return transform_reduce("dot", Multiply(), Add(), lhs, rhs);
}

/**
 * @ingroup reduction
 * @brief Compute the Lp-norm of a vector raised to the power p.
 * @tparam P The power
 */
template <int P = 2, typename TIn>
typename TIn::element_type norm(const TIn& in)
{
  return transform_reduce("norm", Abspow<P>(), Add(), in);
}

/**
 * @ingroup reduction
 * @brief Compute the absolute Lp-distance between two vectors raised to the power p.
 * @tparam P The power
 */
template <int P = 2, typename TLhs, typename TRhs>
typename TLhs::element_type distance(const TLhs& lhs, const TRhs& rhs)
{
  return transform_reduce("distance", Abspow<P>(), Add(), lhs, rhs);
}

} // namespace Linx

#endif
