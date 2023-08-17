// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Base/AlignedBuffer.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(AlignedBuffer_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(default_alignment_test)
{
  AlignedBuffer<int> owner(10);
  BOOST_TEST(owner.alignment_req() % 16 == 0);
  BOOST_TEST(owner.alignment() % 16 == 0);

  AlignedBuffer<const int> view(10, owner.begin());
  BOOST_TEST(view.alignment_req() == 1);
  BOOST_TEST(view.alignment() % 16 == 0);

  AlignedBuffer<const int> aligned_view(10, owner.begin(), -1);
  BOOST_TEST(aligned_view.alignment_req() % 16 == 0);
  BOOST_TEST(aligned_view.alignment() % 16 == 0);
}

BOOST_AUTO_TEST_CASE(alignment_test)
{
  for (std::size_t as = 16; as <= 1024; as <<= 1) {
    AlignedBuffer<int> buffer(10, nullptr, as);
    BOOST_TEST(buffer.alignment_req() == as);
    BOOST_TEST(buffer.alignment() % as == 0);
    AlignedBuffer<int> view(10, const_cast<int*>(buffer.begin()), as);
    BOOST_TEST(view.begin() == buffer.begin());
    AlignedBuffer<const int> c_view(10, buffer.begin(), as);
    BOOST_TEST(c_view.begin() == buffer.begin());
  }
}

BOOST_AUTO_TEST_CASE(release_reset_test)
{
  AlignedBuffer<int> owner(7);
  AlignedBuffer<const int> viewer(7, owner.begin());
  BOOST_TEST(not viewer.release());
  BOOST_TEST(viewer.begin());
  auto* p = owner.release();
  BOOST_TEST(p);
  BOOST_TEST(owner.begin());
  BOOST_TEST(not owner.owns());
  BOOST_CHECK_NO_THROW(std::free(p));
  viewer.reset();
  BOOST_TEST(not viewer.begin());
  owner.reset();
  BOOST_TEST(not owner.begin());
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
