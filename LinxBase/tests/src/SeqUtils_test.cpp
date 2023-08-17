// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Base/SeqUtils.h"

#include <boost/test/unit_test.hpp>
#include <type_traits>

using namespace Linx;

template <typename T>
struct PassBySpy {
  PassBySpy(T v) : value(v), moved(false), copied(false) {}

  PassBySpy(const PassBySpy& p) : value(p.value), moved(false), copied(true) {}

  PassBySpy(PassBySpy&& p) : value(p.value), moved(true), copied(false) {}

  ~PassBySpy() = default;

  PassBySpy& operator=(const PassBySpy& p)
  {
    value = p.value;
    copied = true;
  }

  PassBySpy& operator=(PassBySpy&& p)
  {
    value = p.value;
    moved = true;
  }

  T value;
  bool moved;
  bool copied;
};

struct Body {
  std::string name;
  int age;
  float height;
  float mass;
  float bmi() const
  {
    return mass / (height * height);
  }
};

std::string to_string(std::string name, int age, float height, float mass)
{
  return name + " (" + std::to_string(age) + ") :" + std::to_string(height) + "m, " + std::to_string(mass) + "kg";
}

template <typename TSeq>
void dispatch_seq(TSeq&& seq, bool is_tuple);

template <typename T>
void dispatch_seq(const std::vector<T>& seq, bool is_tuple);

template <typename T>
void dispatch_seq(std::vector<T>&& seq, bool is_tuple);

template <typename TSeq>
void dispatch_seq(TSeq&& seq, bool is_tuple)
{
  seq_foreach(std::forward<TSeq>(seq), [&](const auto& e) {
    (void)e;
    BOOST_TEST(is_tuple);
  });
}

template <typename T>
void dispatch_seq(const std::vector<T>& seq, bool is_tuple)
{
  for (const auto& e : seq) {
    (void)e;
    BOOST_TEST(not is_tuple);
  }
}

template <typename T>
void dispatch_seq(std::vector<T>&& seq, bool is_tuple)
{
  for (const auto& e : seq) {
    (void)e;
    BOOST_TEST(not is_tuple);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(SeqUtils_test)

//-----------------------------------------------------------------------------

LINX_TEST_CASE_TEMPLATE(supported_types_test)
{
  BOOST_TEST(std::is_standard_layout<T>::value);
}

BOOST_AUTO_TEST_CASE(tuple_as_test)
{
  const std::tuple<std::string, int, float, float> tuple {"TODO", 20, 1.8, 75};
  const auto body = tuple_as<Body>(tuple);
  BOOST_TEST(body.name == "TODO");
  BOOST_TEST(body.age == 20);
  BOOST_TEST(body.height > 1.75);
  BOOST_TEST(body.height < 1.85);
  BOOST_TEST(body.mass == 75);
  BOOST_TEST(body.bmi() < 30); // TODO relevant?
}

BOOST_AUTO_TEST_CASE(tuple_apply_test)
{
  std::tuple<std::string, int, float, float> guy {"GUY", 18, 1.7, 55};
  const auto repr = tuple_apply(guy, to_string);
  BOOST_TEST(not repr.empty());
}

BOOST_AUTO_TEST_CASE(tuple_transform_test)
{
  std::tuple<std::string, int, float, float> jo {"JO", 40, 1.6, 85};
  auto twice = [](const auto& e) {
    return e + e;
  };
  const auto jojo = seq_transform<Body>(jo, twice);
  BOOST_TEST(jojo.name == "JOJO");
  BOOST_TEST(jojo.age > std::get<1>(jo));
  BOOST_TEST(jojo.height > std::get<2>(jo));
  BOOST_TEST(jojo.mass > std::get<3>(jo));
}

BOOST_AUTO_TEST_CASE(tuple_foreach_test)
{
  std::tuple<std::string, int, float, float> me {"ME", 32, 1.75, 65};
  auto twice = [](auto& e) {
    e += e;
  };
  seq_foreach(me, twice);
  BOOST_TEST(std::get<0>(me) == "MEME");
  BOOST_TEST(std::get<1>(me) > 32);
  BOOST_TEST(std::get<2>(me) > 1.75);
  BOOST_TEST(std::get<3>(me) > 65);
}

BOOST_AUTO_TEST_CASE(seq_dispatch_test)
{
  const std::tuple<int, float> t {1, 3.14};
  const std::vector<int> v {1, 2};
  dispatch_seq(t, true);
  dispatch_seq(v, false);
  dispatch_seq(std::vector<float> {1, 3.14}, false);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
