// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "Raster/Raster.h"

#include <map>
#include <string>

static Elements::Logging logger = Elements::Logging::getLogger("RasterDemoPixelwise");

class RasterDemoPixelwise : public Elements::Program {

public:
  ExitCode mainMethod(std::map<std::string, VariableValue>& /* args */) override {
    Cnes::VecRaster<int> a({3, 2}); // FIXME , {1, 2, 3, 4, 5, 6});
    Cnes::VecRaster<int> b({3, 2}); // FIXME , -1);
    Cnes::VecRaster<int> c({3, 2}); // FIXME , ?
    int k = 2;
    operators(a, b, k);
    // FIXME functions(a, b, c, k);
    apply(a, b, k);
    return ExitCode::OK;
  }

  template <typename TRaster, typename T>
  void operators(TRaster& a, const TRaster& b, T k) {
    //! [Operators]
    a += b; // Is equivalent to:
    std::transform(a.begin(), a.end(), b.begin(), a.begin(), [](auto e, auto f) {
      return e + f;
    });

    a += k; // Is equivalent to:
    std::transform(a.begin(), a.end(), a.begin(), [=](auto e) {
      return e + k;
    });

    auto res1 = a + b; // Is equivalent to:
    auto res2 = a;
    res2 += b;

    auto res3 = a + k; // Is equivalent to:
    auto res4 = a;
    res4 += k;
    //! [Operators]
  }

  template <typename TRaster, typename T>
  void functions(TRaster& a, const TRaster& b, const URaster& c, T k) {
    //! [Functions]
    a.sin();
    auto res1 = sin(a); // Calls Cnes::sin(a) according to ADL

    a.pow(k);
    auto res2 = pow(a, k);

    auto res3 = norm(c);
    // No in-place version when types do not match:
    // c is complex while res3 is real
    //! [Functions]
  }

  template <typename TRaster, typename T>
  void apply(TRaster& a, const TRaster& b, T k) {

    //! [Formula]
    auto res = a * k + b;
    //! [Formula]

    //! [Generate]
    res.generate(
        [=](auto e, auto f) {
          return e * k + f;
        },
        a,
        b);
    //! [Generate]

    //! [Apply]
    a.apply(
        [=](auto e, auto f) {
          return e * k + f;
        },
        b);
    //! [Apply]
  }
};

MAIN_FOR(RasterDemoPixelwise)
