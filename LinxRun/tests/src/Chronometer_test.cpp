// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Run/Chronometer.h"

#include <boost/test/unit_test.hpp>
#include <thread> // sleep_for

using namespace Linx;

struct ChronoFixture : public Chronometer<std::chrono::milliseconds> {
  ChronoFixture(std::chrono::milliseconds chrono_offset = std::chrono::milliseconds {std::rand()}) :
      Chronometer<std::chrono::milliseconds>(chrono_offset), offset(chrono_offset)
  {}
  void wait(std::int64_t ms = default_wait)
  {
    std::this_thread::sleep_for(Unit(ms));
  }
  Unit offset;
  static constexpr std::int64_t default_wait = 10;
};

constexpr std::int64_t ChronoFixture::default_wait;

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(Chronometer_test, ChronoFixture)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(init_test)
{
  BOOST_TEST(elapsed().count() == offset.count());
  BOOST_TEST(not is_running());
  BOOST_TEST(size() == 0);
}

BOOST_AUTO_TEST_CASE(one_inc_test)
{
  start();
  BOOST_TEST(is_running());
  wait();
  stop();
  BOOST_TEST(not is_running());
  BOOST_TEST(elapsed().count() >= offset.count());
  BOOST_TEST(size() == 1);
  const auto inc = last().count();
  BOOST_TEST(inc >= default_wait);
  BOOST_TEST(elapsed().count() == offset.count() + inc);
  BOOST_TEST(distribution().mean() == inc);
  BOOST_TEST(distribution().stdev(false) == 0.); // Exactly 0.
  BOOST_TEST(min() == inc);
  BOOST_TEST(max() == inc);
}

BOOST_AUTO_TEST_CASE(two_incs_test)
{
  start();
  wait(); // Wait
  stop();
  start();
  BOOST_TEST(is_running());
  wait(default_wait * 10); // Wait more
  stop();
  BOOST_TEST(not is_running());
  BOOST_TEST(elapsed().count() > offset.count());
  BOOST_TEST(size() == 2);
  const auto fast = increments()[0];
  const auto slow = increments()[1];
  BOOST_TEST(fast < slow); // FIXME Not that sure!
  BOOST_TEST(elapsed().count() == offset.count() + fast + slow);
  BOOST_TEST(distribution().mean() >= fast);
  BOOST_TEST(distribution().mean() <= slow);
  BOOST_TEST(distribution().stdev() > 0.);
  BOOST_TEST(min() == fast);
  BOOST_TEST(max() == slow);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
