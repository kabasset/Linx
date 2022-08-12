// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LitlRaster/BorderedBox.h"

#include <boost/test/unit_test.hpp>
#include <set>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(BorderedBox_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(position_set_test) {

  const auto inner = Box<2>::fromShape({1, 1}, {4, 3});
  const Box<2> margin {{-3, -2}, {2, 1}};
  const auto box = inner + margin;
  const BorderedBox<2> bordered(box, margin);

  std::set<Indices<2>> innerSet;
  for (const auto& p : inner) {
    innerSet.insert(p.container());
  }
  std::set<Indices<2>> all;
  for (const auto& p : box) {
    all.insert(p.container());
  }

  BOOST_TEST(all.size() == box.size());

  std::set<Indices<2>> outInner;
  std::set<Indices<2>> outAll;
  bordered.applyInnerBorder(
      [&](const auto& b) {
        for (const auto& p : b) {
          outInner.insert(p.container());
          outAll.insert(p.container());
        }
      },
      [&](const auto& b) {
        for (const auto& p : b) {
          outAll.insert(p.container());
        }
      });

  BOOST_TEST(outInner == innerSet);
  BOOST_TEST(outAll == all);
}

BOOST_AUTO_TEST_CASE(box_ordering_test) {

  const auto inner = Box<2>::fromShape({1, 1}, {4, 3});
  const Box<2> margin {{-3, -2}, {2, 1}};
  const BorderedBox<2> bordered(inner + margin, margin);

  std::vector<Box<2>> expected {
      {{-2, -1}, {6, 0}}, // top
      {{-2, 1}, {0, 3}}, // left
      inner,
      {{5, 1}, {6, 3}}, // right
      {{-2, 4}, {6, 4}} // bottom
  };
  std::vector<Box<2>> out;
  bordered.applyInnerBorder(
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
