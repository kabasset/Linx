// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "RasterFourier/Dft.h"

#include <map>
#include <string>

void filter() {

  //! [Plan direct]
  Cnes::RealDft direct({640, 480});
  //! [Plan direct]
  std::cout << "Input buffer after planning: " << direct.in()[0] << std::endl;
  std::cout << "Output buffer after planning: " << direct.out()[0] << std::endl;

  //! [Plan inverse]
  auto inverse = direct.inverse();
  // inverse is of type RealDft::Inverse
  // inverse.in() is real.out() and inverse.out() is real.in()!
  //! [Plan inverse]
  std::cout << "Input buffer after inverse planning: " << direct.in()[0] << std::endl;
  std::cout << "Output buffer after inverse planning: " << direct.out()[0] << std::endl;

  //! [Fill signal]
  auto& signal = direct.in();
  for (const auto& p : signal.domain()) {
    signal[p] = 1 + p[0] + p[1];
  }
  //! [Fill signal]

  //! [Transfer function]
  Cnes::ComplexDftBuffer transfer(direct.outShape());
  std::fill(transfer.begin(), transfer.end(), std::complex<double> {2., 3.});
  //! [Transfer function]

  //! [Direct transform]
  direct.transform();
  // Transformed data is in direct.out(), direct.in() is garbage now
  direct.out() *= transfer;
  // Filtered data is in direct.out()
  //! [Direct transform]

  //! [Inverse transform]
  inverse.transform().normalize();
  // Inverse transformed data is in direct.in(), direct.out() is garbage now
  //! [Inverse transform]

  std::cout << "Input buffer after filtering: " << signal[0] << std::endl;
}

void multithread() {}

class RasterDemoDft : public Elements::Program {

public:
  ExitCode mainMethod(std::map<std::string, VariableValue>& /* args */) override {
    filter();
    multithread();
    return ExitCode::OK;
  }
};

MAIN_FOR(RasterDemoDft)
