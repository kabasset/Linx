// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Run/ProgramOptions.h"

#include <boost/test/unit_test.hpp>

using namespace Linx;

struct TestProgramOptions {
  ProgramOptions po;

  TestProgramOptions() : po("Description")
  {
    po.positional<int>("positional", "Positional option, no short form, no default");
    po.positional("positional-default", "Positional option with short form and default", 0);
    po.named<char>("named", "Named option, no short form, no default");
    po.named<char>("named-short,n", "Named option with short form, no default");
    po.named("named-default,d", "Named option with short form and default", 'd');
    po.flag("flag", "Flag, no short form");
    po.flag("flag-short,f", "Flag with short form");
  }
};

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(ProgramOptionsTest, TestProgramOptions)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(positional_test)
{
  po.parse("exe 1 --named a -n b");
  BOOST_TEST(po.as<int>("positional") == 1);
  BOOST_TEST(po.as<int>("positional-default") == 0);
  po.parse("exe 1 2 --named a -n b");
  BOOST_TEST(po.as<int>("positional") == 1);
  BOOST_TEST(po.as<int>("positional-default") == 2);
}

BOOST_AUTO_TEST_CASE(named_test)
{
  po.parse("exe 1 --named a -n b");
  BOOST_TEST(po.as<char>("named") == 'a');
  BOOST_TEST(po.as<char>("named-short") == 'b');
  BOOST_TEST(po.as<char>("named-default") == 'd');
  po.parse("exe 1 --named a -n b -d c");
  BOOST_TEST(po.as<char>("named-default") == 'c');
}

BOOST_AUTO_TEST_CASE(flag_test)
{
  po.parse("exe 1 --named a -n b");
  BOOST_TEST(not po.as<bool>("flag"));
  BOOST_TEST(not po.has("flag"));
  BOOST_TEST(not po.as<bool>("flag-short"));
  BOOST_TEST(not po.has("flag-short"));
  po.parse("exe 1 --named a -n b --flag");
  BOOST_TEST(po.as<bool>("flag"));
  BOOST_TEST(po.has("flag"));
  po.parse("exe 1 --named a -n b -f");
  BOOST_TEST(po.as<bool>("flag-short"));
  BOOST_TEST(po.has("flag-short"));
}

BOOST_AUTO_TEST_CASE(bad_options_test)
{
  BOOST_CHECK_THROW(po.parse("missing_positional --named a -n b"), std::exception);
  BOOST_CHECK_THROW(po.parse("missing_named 1 -n b"), std::exception);
  BOOST_CHECK_THROW(po.parse("wrong_type 1 --named=str -n b"), std::exception);
  BOOST_CHECK_THROW(po.parse("unknown_option 1 --named a -n b --bad 0"), std::exception);
  BOOST_CHECK_THROW(po.parse("spurious_arg 1 --named a -n b -f 1"), std::exception);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
