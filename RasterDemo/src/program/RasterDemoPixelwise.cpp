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
    Cnes::VecRaster<double> a({3, 2}); // FIXME , {1, 2, 3, 4, 5, 6});
    Cnes::VecRaster<double> b({3, 2}); // FIXME , -1);
    Cnes::VecRaster<std::complex<double>> c({3, 2}); // FIXME , ?
    double k = 2;
    operators(a, b, k);
    functions(a, b, c, k);
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

  template <typename TRaster, typename URaster, typename T>
  void functions(TRaster& a, const TRaster& b, const URaster& c, T k) {
    //! [Functions]
    // Unary function
    auto res1 = cos(a); // Calls Cnes::cos(a) according to ADL
    a.cos();

    // Binary function with scalar second argument
    auto res2 = pow(a, k);
    a.pow(k);

    // Binary function with raster second argument
    auto res3 = pow(a, b);
    a.pow(b);

    auto res4 = norm(c);
    // No in-place version when types do not match:
    // c is complex while res4 is real
    //! [Functions]
    std::cout << "cos(a) = " << res1[0] << std::endl;
    std::cout << "pow(a, k) = " << res2[0] << std::endl;
    std::cout << "pow(a, b) = " << res3[0] << std::endl;
    std::cout << "norm(c) = " << res4[0] << std::endl;
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
