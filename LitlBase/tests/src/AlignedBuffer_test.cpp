// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlBase/AlignedBuffer.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(AlignedBuffer_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(default_alignment_test) {
  AlignedBuffer<int> owner(10);
  BOOST_TEST(owner.alignmentReq() % 16 == 0);
  BOOST_TEST(owner.alignment() % 16 == 0);

  AlignedBuffer<const int> view(10, owner.data());
  BOOST_TEST(view.alignmentReq() == 1);
  BOOST_TEST(view.alignment() % 16 == 0);

  AlignedBuffer<const int> alignedView(10, owner.data(), -1);
  BOOST_TEST(alignedView.alignmentReq() % 16 == 0);
  BOOST_TEST(alignedView.alignment() % 16 == 0);
}

BOOST_AUTO_TEST_CASE(alignment_test) {
  for (std::size_t as = 16; as <= 1024; as <<= 1) {
    AlignedBuffer<int> buffer(10, nullptr, as);
    BOOST_TEST(buffer.alignmentReq() == as);
    BOOST_TEST(buffer.alignment() % as == 0);
    AlignedBuffer<int> view(10, const_cast<int*>(buffer.data()), as);
    BOOST_TEST(view.data() == buffer.data());
    AlignedBuffer<const int> cView(10, buffer.data(), as);
    BOOST_TEST(cView.data() == buffer.data());
  }
}

BOOST_AUTO_TEST_CASE(release_reset_test) {
  AlignedBuffer<int> owner(7);
  AlignedBuffer<const int> viewer(7, owner.data());
  BOOST_TEST(not viewer.release());
  BOOST_TEST(viewer.data());
  auto* p = owner.release();
  BOOST_TEST(p);
  BOOST_TEST(owner.data());
  BOOST_TEST(not owner.owns());
  BOOST_CHECK_NO_THROW(std::free(p));
  viewer.reset();
  BOOST_TEST(not viewer.data());
  owner.reset();
  BOOST_TEST(not owner.data());
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
