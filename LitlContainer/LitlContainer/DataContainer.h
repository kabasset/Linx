// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLCONTAINER_DATACONTAINER_H
#define _LITLCONTAINER_DATACONTAINER_H

#include "LitlContainer/Arithmetic.h"
#include "LitlContainer/ContiguousContainer.h"
#include "LitlContainer/DataDistribution.h"
#include "LitlContainer/Holders.h"
#include "LitlContainer/Math.h"
#include "LitlContainer/Range.h"
#include "LitlTypes/Exceptions.h"
#include "LitlTypes/SeqUtils.h" // isIterable
#include "LitlTypes/TypeUtils.h" // Index, Limits

#include <cstddef> // size_t
#include <initializer_list>
#include <tuple>
#include <type_traits> // enable_if, is_integral, decay
#include <utility> // forward

namespace Litl {

/**
 * @ingroup data_classes
 * @brief Combine a `ContiguousContainerMixin` and `DataHolder` as a user-defined derived class.
 * @tparam T The element type
 * @tparam THolder The data container holder
 * @tparam TDerived The derived class
 * @details
 * The class can be specialized for any container which implements the `SizedData` requirements.
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

  /// @{
  /// @group_construction

  /**
   * @brief Size-based constructor.
   */
  template <typename... TArgs>
  explicit DataContainer(std::size_t s = 0, TArgs&&... args) : Holder(s, std::forward<TArgs>(args)...) {}

  /**
   * @brief List-based constructor.
   * @details
   * List values are copied to the container.
   */
  template <typename U, typename... TArgs>
  explicit DataContainer(std::initializer_list<U> list, TArgs&&... args) :
      Holder(list.size(), std::forward<TArgs>(args)...) {
    std::copy(list.begin(), list.end(), this->data());
  }

  /**
   * @brief Iterator-based constructor.
   * @details
   * Iterable values are copied to the container.
   */
  template <typename TIterable, typename std::enable_if_t<isIterable<TIterable>::value>* = nullptr, typename... TArgs>
  explicit DataContainer(const TIterable& iterable, TArgs&&... args) :
      Holder(std::distance(iterable.begin(), iterable.end()), std::forward<TArgs>(args)...) {
    std::copy(iterable.begin(), iterable.end(), this->data());
  }

  LITL_VIRTUAL_DTOR(DataContainer)
  LITL_DEFAULT_COPYABLE(DataContainer)
  LITL_DEFAULT_MOVABLE(DataContainer)

  /// @group_properties

  /**
   * @brief Inherit data holder's `size()`.
   */
  using Holder::size;

  /// @group_elements

  /**
   * @brief Inherit data holder's `data()`.
   */
  using Holder::data;

  /**
   * @brief Access the raw data.
   */
  inline T* data() {
    return const_cast<T*>(const_cast<const DataContainer&>(*this).data());
  }

  /**
   * @brief Access the element with given index.
   * @details
   * As opposed to `operator[]()`, negative indices are supported for backward indexing,
   * and bounds are checked.
   */
  const T& at(Index i) const {
    const auto s = size();
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

} // namespace Litl

#endif
