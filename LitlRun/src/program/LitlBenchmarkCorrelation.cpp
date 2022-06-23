// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "LitlRun/ProgramOptions.h"
#include "LitlTransforms/Kernel1d.h"

#include <map>
#include <string>

static Elements::Logging logger = Elements::Logging::getLogger("LitlBenchmarkCorrelation");

class RasterBenchmarkCorrelation : public Elements::Program {

public:
  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override {
    Litl::ProgramOptions options;
    options.positional("setup", "Test case setup", std::string("separable"));
    options.named("side", "Raster length along each axis", 100L);
    return options.asPair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override {
    const auto side = args["side"].as<Litl::Index>();
    Litl::Raster<int, 4> in({side, side, side, side});
    const auto kernel = Litl::sobel<int, 0, 1>();
    const auto out = kernel.correlate<int>(in);
    return ExitCode::OK;
  }
};

MAIN_FOR(RasterBenchmarkCorrelation)
