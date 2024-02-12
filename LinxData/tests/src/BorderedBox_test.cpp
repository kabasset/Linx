// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Data/BorderedBox.h"

#include <boost/test/unit_test.hpp>
#include <set>

using namespace Linx;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(BorderedBox_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(position_set_test)
{
  const auto inner = Box<2>::from_shape({1, 1}, {4, 3});
  const Box<2> margin {{-3, -2}, {2, 1}};
  const auto box = inner + margin;
  const Internal::BorderedBox<2> bordered(box, margin);

  std::set<Indices<2>> inner_set;
  for (const auto& p : inner) {
    inner_set.insert(p.container());
  }
  std::set<Indices<2>> all;
  for (const auto& p : box) {
    all.insert(p.container());
  }

  BOOST_TEST(all.size() == box.size());

  std::set<Indices<2>> out_inner;
  std::set<Indices<2>> out_all;
  bordered.apply_inner_border(
      [&](const auto& b) {
        for (const auto& p : b) {
          out_inner.insert(p.container());
          out_all.insert(p.container());
        }
      },
      [&](const auto& b) {
        for (const auto& p : b) {
          out_all.insert(p.container());
        }
      });

  BOOST_TEST(out_inner == inner_set);
  BOOST_TEST(out_all == all);
}

BOOST_AUTO_TEST_CASE(box_ordering_test)
{
  const auto inner = Box<2>::from_shape({1, 1}, {4, 3});
  const Box<2> margin {{-3, -2}, {2, 1}};
  const Internal::BorderedBox<2> bordered(inner + margin, margin);

  std::vector<Box<2>> expected {
      {{-2, -1}, {6, 0}}, // top
      {{-2, 1}, {0, 3}}, // left
      inner,
      {{5, 1}, {6, 3}}, // right
      {{-2, 4}, {6, 4}} // bottom
  };
  std::vector<Box<2>> out;
  bordered.apply_inner_border(
      [&](const auto& b) {
        out.push_back(b);
      },
      [&](const auto& b) {
        out.push_back(b);
      });

  BOOST_TEST(out == expected);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
