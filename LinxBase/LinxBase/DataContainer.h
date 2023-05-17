// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXBASE_DATACONTAINER_H
#define _LINXBASE_DATACONTAINER_H

#include "LinxBase/Arithmetic.h"
#include "LinxBase/ContiguousContainer.h"
#include "LinxBase/DataDistribution.h"
#include "LinxBase/Exceptions.h"
#include "LinxBase/Holders.h"
#include "LinxBase/Math.h"
#include "LinxBase/Range.h"
#include "LinxBase/SeqUtils.h" // IsRange
#include "LinxBase/TypeUtils.h" // Index, Limits

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

  /**
   * @brief Size-based constructor.
   */
  template <typename... TArgs>
  explicit DataContainer(std::size_t s = 0, TArgs&&... args) : Holder(s, std::forward<TArgs>(args)...) {}

  /**
   * @brief Iterator-based constructor.
   * 
   * Iterated values are copied to the container.
   */
  template <typename TIt, typename... TArgs>
  explicit DataContainer(TIt begin, TIt end, TArgs&&... args) :
      DataContainer(std::distance(begin, end), std::forward<TArgs>(args)...) {
    std::copy(std::move(begin), std::move(end), this->data());
  }

  /**
   * @brief List-based constructor.
   * 
   * List values are copied to the container.
   */
  template <typename... TArgs>
  explicit DataContainer(std::initializer_list<T> list, TArgs&&... args) :
      DataContainer(list.begin(), list.end(), std::forward<TArgs>(args)...) {}

  /**
   * @brief Range-based constructor.
   * 
   * Range values are copied to the container.
   */
  template <typename TRange, typename std::enable_if_t<IsRange<TRange>::value>* = nullptr, typename... TArgs>
  explicit DataContainer(const TRange& range, TArgs&&... args) :
      DataContainer(range.begin(), range.end(), std::forward<TArgs>(args)...) {}

  LINX_VIRTUAL_DTOR(DataContainer)
  LINX_DEFAULT_COPYABLE(DataContainer)
  LINX_DEFAULT_MOVABLE(DataContainer)

  /// @group_elements

  /**
   * @brief Access the element with given index.
   * 
   * As opposed to `operator[]()`, negative indices are supported for backward indexing,
   * and bounds are checked.
   */
  const T& at(Index i) const {
    const auto s = this->size();
    OutOfBoundsError::mayThrow("Index " + std::to_string(i), i, {-s, s - 1});
    return this->operator[](i >= 0 ? i : i + s);
  }

  /**
   * @copybrief at()
   */
  T& at(Index i) {
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
  explicit MinimalDataContainer(std::size_t size = 0, TArgs&&... args) : Base(size, std::forward<TArgs>(args)...) {}

  template <typename U>
  MinimalDataContainer(std::initializer_list<U> list) : Base(list) {}

  template <typename U, typename... TArgs>
  explicit MinimalDataContainer(std::initializer_list<U> list, TArgs&&... args) :
      Base(list, std::forward<TArgs>(args)...) {}

  template <typename TRange, typename std::enable_if_t<IsRange<TRange>::value>* = nullptr, typename... TArgs>
  explicit MinimalDataContainer(TRange& range, TArgs&&... args) : Base(range, std::forward<TArgs>(args)...) {}
};

} // namespace Linx

#endif
