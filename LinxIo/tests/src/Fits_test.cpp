/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Base/TypeUtils.h"
#include "Linx/Io.h"

#include <boost/test/unit_test.hpp>
#include <fstream>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Fits_test)

//-----------------------------------------------------------------------------

using SupportedFitsTypes = std::tuple<
    char,
    signed char,
    unsigned char,
    short,
    unsigned short,
    int,
    unsigned int,
    long,
    unsigned long,
    long long,
    unsigned long long,
    float,
    double>;

BOOST_AUTO_TEST_CASE_TEMPLATE(fits_write_read_test, T, SupportedFitsTypes) { // FIXME for several Ns
  Raster<T> in({16, 16});
  in.range();
  std::string name("BITPIX");
  name += std::to_string(Fits::bitpix<T>());
  name += std::is_signed_v<T> ? 's' : 'u';
  name += typeid(T).name();
  name += ".fits";
  TemporaryPath path(name);
  Fits io(path);
  io.write(in);
  BOOST_TEST(std::filesystem::exists(io.path()));
  const auto out = io.read<Raster<T>>();
  BOOST_TEST(out == in);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(auto_write_read_test, T, SupportedFitsTypes) {
  Raster<T> in({16, 16});
  in.range();
  std::string name("BITPIX");
  name += std::to_string(Fits::bitpix<T>());
  name += std::is_signed_v<T> ? 's' : 'u';
  name += typeid(T).name();
  name += ".fits";
  TemporaryPath path(name);
  write(in, path);
  const auto out = read<T>(path);
  BOOST_TEST(out == in);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(auto_append_read_test, T, SupportedFitsTypes) {
  Raster<T> in({16, 16});
  in.range();
  std::string name("BITPIX");
  name += std::to_string(Fits::bitpix<T>());
  name += std::is_signed_v<T> ? 's' : 'u';
  name += typeid(T).name();
  name += ".fits";
  TemporaryPath path(name);
  write(Raster<int, 0>(), path); // Empty Primary
  write(in, path, 'a');
  const auto out = read<T>(path, 1);
  BOOST_TEST(out == in);
}

BOOST_AUTO_TEST_CASE(auto_read_wrong_format_test) {
  TemporaryPath path("dummy.txt"); // FIXME .fits
  BOOST_ASSERT(not std::filesystem::exists(path));
  std::ofstream f(path.string());
  f << "DUMMY";
  f.close();
  BOOST_CHECK_THROW(read<int>(path), FileFormatError);
}

BOOST_AUTO_TEST_CASE(auto_read_missing_file_test) {
  BOOST_CHECK_THROW(read<int>("no_such_file.fits"), FileNotFoundError);
}

BOOST_AUTO_TEST_CASE(auto_write_mode_errors_test) {
  TemporaryPath path("dummy.fits");
  BOOST_CHECK_THROW(write(Raster<int, 1>({1}), path, 'a'), FileNotFoundError);
  std::ofstream f(path.string());
  f << "DUMMY";
  f.close();
  BOOST_CHECK_THROW(write(Raster<int, 1>({1}), path), PathExistsError);
  BOOST_CHECK_NO_THROW(write(Raster<int, 1>({1}), path, 'w'));
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
