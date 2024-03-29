// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXBASE_MIXINS_DATACONTAINER_H
#define _LINXBASE_MIXINS_DATACONTAINER_H

#include "Linx/Base/DataDistribution.h"
#include "Linx/Base/Exceptions.h"
#include "Linx/Base/Holders.h"
#include "Linx/Base/SeqUtils.h" // IsRange
#include "Linx/Base/TypeUtils.h" // Index, Limits
#include "Linx/Base/mixins/Arithmetic.h"
#include "Linx/Base/mixins/ContiguousContainer.h"
#include "Linx/Base/mixins/Math.h"
#include "Linx/Base/mixins/Range.h"

#include <cstddef> // size_t
#include <initializer_list>
#include <tuple>
#include <type_traits> // enable_if, is_integral, decay
#include <utility> // forward

namespace Linx {

/**
 * @ingroup data_classes
 * @brief Combine a `ContiguousContainerMixin` and `DataHolder` as a user-defined derived class.
 * @tparam T The element type
 * @tparam THolder The data container holder
 * @tparam TDerived The derived class
 * 
 * The class can be specialized for any container which implements the `ContiguousRange` requirements.
 * @satisfies{ContiguousContainer}
 */
template <typename T, typename THolder, typename TArithmetic, typename TDerived>
class DataContainer :
    public THolder,
    public ContiguousContainerMixin<T, TDerived>,
    public ArithmeticMixin<TArithmetic, T, TDerived>,
    public MathFunctionsMixin<T, TDerived>,
    public RangeMixin<T, TDerived> {
public:

  /**
   * @brief The concrete data holder type.
   */
  using Holder = THolder;

  using Holder::begin;
  using Holder::end;
  using ContiguousContainerMixin<T, TDerived>::begin;
  using ContiguousContainerMixin<T, TDerived>::end;

  /// @{
  /// @group_construction

  LINX_VIRTUAL_DTOR(DataContainer)
  LINX_DEFAULT_COPYABLE(DataContainer)
  LINX_DEFAULT_MOVABLE(DataContainer)

  /**
   * @brief Size-based constructor.
   */
  template <typename... TArgs>
  explicit DataContainer(std::size_t s = 0, TArgs&&... args) : Holder(s, LINX_FORWARD(args)...)
  {}

  /**
   * @brief Iterator-based constructor.
   * 
   * Iterated values are copied to the container.
   */
  template <typename TIt, typename... TArgs>
  explicit DataContainer(TIt begin, TIt end, TArgs&&... args) :
      DataContainer(std::distance(begin, end), LINX_FORWARD(args)...)
  { // FIXME distance might be expensive
    std::copy(begin, end, this->data());
  }

  /// @group_elements

  /**
   * @brief Access the element with given index.
   * 
   * As opposed to `operator[]()`, negative indices are supported for backward indexing,
   * and bounds are checked.
   */
  const T& at(Index i) const
  {
    const auto s = this->size();
    OutOfBoundsError::may_throw("Index " + std::to_string(i), i, {-s, s - 1});
    return this->operator[](i >= 0 ? i : i + s);
  }

  /**
   * @copybrief at()
   */
  T& at(Index i)
  {
    return const_cast<T&>(const_cast<const DataContainer&>(*this).at(i));
  }

  /// @}
};

/**
 * @brief A minimal `DataContainer` class, mostly for testing purpose.
 * 
 * `Sequence` and `Vector` are better-suited for general purpose usage.
 */
template <typename T>
class MinimalDataContainer : public DataContainer<T, DefaultHolder<T>, void, MinimalDataContainer<T>> {
public:

  using Base = DataContainer<T, DefaultHolder<T>, void, MinimalDataContainer<T>>;

  template <typename... TArgs>
  explicit MinimalDataContainer(std::size_t size = 0, TArgs&&... args) : Base(size, LINX_FORWARD(args)...)
  {}

  template <typename U>
  MinimalDataContainer(std::initializer_list<U> list) : Base(list.begin(), list.end())
  {}

  template <typename U, typename... TArgs>
  explicit MinimalDataContainer(std::initializer_list<U> list, TArgs&&... args) :
      Base(list.begin(), list.end(), LINX_FORWARD(args)...)
  {}

  template <typename TRange, typename std::enable_if_t<IsRange<TRange>::value>* = nullptr, typename... TArgs>
  explicit MinimalDataContainer(TRange& range, TArgs&&... args) :
      Base(range.begin(), range.end(), LINX_FORWARD(args)...)
  {}
};

} // namespace Linx

#endif
