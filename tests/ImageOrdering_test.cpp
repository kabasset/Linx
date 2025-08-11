// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE ImageOrderingTest

#include "Linx/Base/mixins/Strided.h"
#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

template <typename TIn, typename TTest>
struct CompareValues {
  TIn m_in;
  TTest m_test;
  KOKKOS_INLINE_FUNCTION void operator()(auto... is) const
  {
    auto value = m_in.front() + static_cast<typename TIn::element_type>(Linx::offset_from_origin(m_in, is...));
    m_test(is...) = (value == m_in(is...));
  }
};

template <typename TIn, typename TTest>
struct CompareAddresses {
  TIn m_in;
  TTest m_test;
  KOKKOS_INLINE_FUNCTION void operator()(auto... is) const
  {
    auto ptr = &m_in.front() + static_cast<typename TIn::element_type>(Linx::offset_from_origin(m_in, is...));
    m_test(is...) = (ptr == &m_in(is...));
  }
};

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(address_test)
{
  auto in = Linx::no_init<Linx::Index>("in", 1, 2, 3, 4, 5, 6).generate_offsets();
  auto test = Linx::default_init<Linx::Index>("test", in.domain());

  Linx::for_each("test values", in.domain(), CompareValues {in, test});
  BOOST_TEST(Linx::sum(test) == test.size());

  Linx::for_each("test addresses", in.domain(), CompareAddresses {in, test});
  BOOST_TEST(Linx::sum(test) == test.size());
}

BOOST_AUTO_TEST_CASE(stride_test)
{
  auto in = Linx::no_init<bool>("in", 1, 2, 3, 4, 5, 6, 7, 8);
  auto strides = in.strides();
  BOOST_TEST(Linx::offset_from_origin(in, 8, 0, 0, 0, 0, 0, 0, 0) == 8 * strides[0]);
  BOOST_TEST(Linx::offset_from_origin(in, 0, 7, 0, 0, 0, 0, 0, 0) == 7 * strides[1]);
  BOOST_TEST(Linx::offset_from_origin(in, 0, 0, 6, 0, 0, 0, 0, 0) == 6 * strides[2]);
  BOOST_TEST(Linx::offset_from_origin(in, 0, 0, 0, 5, 0, 0, 0, 0) == 5 * strides[3]);
  BOOST_TEST(Linx::offset_from_origin(in, 0, 0, 0, 0, 4, 0, 0, 0) == 4 * strides[4]);
  BOOST_TEST(Linx::offset_from_origin(in, 0, 0, 0, 0, 0, 3, 0, 0) == 3 * strides[5]);
  BOOST_TEST(Linx::offset_from_origin(in, 0, 0, 0, 0, 0, 0, 2, 0) == 2 * strides[6]);
  BOOST_TEST(Linx::offset_from_origin(in, 0, 0, 0, 0, 0, 0, 0, 1) == 1 * strides[7]);
  BOOST_TEST(Linx::offset_from_origin(in, -1, -1, -1, -1, -1, -1, -1, -1) == -Linx::sum(strides));
}

BOOST_AUTO_TEST_CASE(shifted_stride_test)
{
  using namespace Linx::Literals;

  auto in = Linx::no_init<bool>("in", Linx::cube<3_D, 1>());
  auto strides = in.strides();
  BOOST_TEST(Linx::offset_from_origin(in, -1, 0, 0) == -strides[0]);
  BOOST_TEST(Linx::offset_from_origin(in, 0, -1, 0) == -strides[1]);
  BOOST_TEST(Linx::offset_from_origin(in, 0, 0, -1) == -strides[2]);
  BOOST_TEST(Linx::offset_from_origin(in, 1, 1, 1) == Linx::sum(strides));
}

BOOST_AUTO_TEST_SUITE_END()
