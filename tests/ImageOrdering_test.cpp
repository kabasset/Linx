// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ImageOrderingTest

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

template <typename TIn, typename TTest>
struct CompareValues {
  TIn m_in;
  TTest m_test;
  KOKKOS_INLINE_FUNCTION void operator()(auto... is) const
  {
    m_test(is...) = (m_in.front() + m_in.offset(is...) == m_in(is...));
  }
};

template <typename TIn, typename TTest>
struct CompareAddresses {
  TIn m_in;
  TTest m_test;
  KOKKOS_INLINE_FUNCTION void operator()(auto... is) const
  {
    m_test(is...) = (&m_in.front() + m_in.offset(is...) == &m_in(is...));
  }
};

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(offset_test)
{
  auto in = Linx::Image<Linx::Index, 6>("in", 1, 2, 3, 4, 5, 6).fill_with_offsets();
  auto test = Linx::Image<Linx::Index, 6>("test", in.shape());

  Linx::for_each("test values", in.domain(), CompareValues {in, test});
  BOOST_TEST(Linx::sum(test) == test.size());

  Linx::for_each("test addresses", in.domain(), CompareAddresses {in, test});
  BOOST_TEST(Linx::sum(test) == test.size());
}

BOOST_AUTO_TEST_CASE(stride_test)
{
  auto in = Linx::Image<bool, 8>("in", 1, 2, 3, 4, 5, 6, 7, 8);
  auto strides = in.strides();
  BOOST_TEST(in.offset(8, 0, 0, 0, 0, 0, 0, 0) == 8 * strides[0]);
  BOOST_TEST(in.offset(0, 7, 0, 0, 0, 0, 0, 0) == 7 * strides[1]);
  BOOST_TEST(in.offset(0, 0, 6, 0, 0, 0, 0, 0) == 6 * strides[2]);
  BOOST_TEST(in.offset(0, 0, 0, 5, 0, 0, 0, 0) == 5 * strides[3]);
  BOOST_TEST(in.offset(0, 0, 0, 0, 4, 0, 0, 0) == 4 * strides[4]);
  BOOST_TEST(in.offset(0, 0, 0, 0, 0, 3, 0, 0) == 3 * strides[5]);
  BOOST_TEST(in.offset(0, 0, 0, 0, 0, 0, 2, 0) == 2 * strides[6]);
  BOOST_TEST(in.offset(0, 0, 0, 0, 0, 0, 0, 1) == 1 * strides[7]);
  BOOST_TEST(in.offset(-1, -1, -1, -1, -1, -1, -1, -1) == -Linx::sum(strides));
}

BOOST_AUTO_TEST_SUITE_END()
