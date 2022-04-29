// Copyright (C) 2022, Antoine Basset
// This file is part of Kast.Raster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "KRasterFilter/Kernel1d.h"
#include "KRasterUtils/ProgramOptions.h"

#include <map>
#include <string>

static Elements::Logging logger = Elements::Logging::getLogger("KBenchmarkCorrelation");

class KBenchmarkCorrelation : public Elements::Program {

public:
  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override {
    Kast::ProgramOptions options;
    options.positional("setup", "Test case setup", std::string("separable"));
    options.named("side", "Raster length along each axis", 100L);
    return options.asPair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override {
    const auto side = args["side"].as<Kast::Index>();
    Kast::VecRaster<int, 4> in({side, side, side, side});
    const auto kernel = Kast::makeSobel<int, 0, 1>();
    const auto out = kernel.correlate<int>(in);
    return ExitCode::OK;
  }
};

MAIN_FOR(KBenchmarkCorrelation)