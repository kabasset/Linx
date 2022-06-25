// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlTypes/TypeUtils.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TypeUtils_test)

//-----------------------------------------------------------------------------

template <typename T>
void checkTypeTraits(T) {
  using Floating = typename TypeTraits<T>::Floating;
  using Scalar = typename TypeTraits<T>::Scalar;
  BOOST_TEST(std::is_floating_point<Floating>::value);
  BOOST_TEST(Limits<Floating>::min() <= Limits<T>::min());
  BOOST_TEST(Limits<Floating>::max() >= Limits<T>::max());
  BOOST_TEST(std::is_scalar<Scalar>::value);
}

template <typename T>
void checkTypeTraits(std::complex<T>) {
  using Scalar = typename TypeTraits<T>::Scalar;
  BOOST_TEST((std::is_same<Scalar, T>::value));
  checkTypeTraits(T());
}

LITL_TEST_CASE_TEMPLATE(type_traits_test) {
  checkTypeTraits(T());
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
