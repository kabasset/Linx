// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Base/TypeUtils.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TypeUtils_test)

//-----------------------------------------------------------------------------

template <typename T>
void check_type_traits(T)
{
  using Floating = typename TypeTraits<T>::Floating;
  using Scalar = typename TypeTraits<T>::Scalar;
  BOOST_TEST(std::is_floating_point<Floating>::value);
  BOOST_TEST(Limits<Floating>::min() <= Limits<T>::min());
  BOOST_TEST(Limits<Floating>::max() >= Limits<T>::max());
  BOOST_TEST(std::is_scalar<Scalar>::value);
}

template <typename T>
void check_type_traits(std::complex<T>)
{
  using Scalar = typename TypeTraits<T>::Scalar;
  BOOST_TEST((std::is_same<Scalar, T>::value));
  check_type_traits(T());
}

LINX_TEST_CASE_TEMPLATE(type_traits_test)
{
  check_type_traits(T());
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
