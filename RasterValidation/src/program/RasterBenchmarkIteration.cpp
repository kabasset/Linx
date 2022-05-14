// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "RasterValidation/IterationBenchmark.h"
#include "RasterValidation/ProgramOptions.h"

#include <map>
#include <string>

Elements::Logging logger = Elements::Logging::getLogger("RasterBenchmarkIteration");

Cnes::IterationBenchmark::Duration iterate(Cnes::IterationBenchmark& benchmark, char setup) {
  switch (setup) {
    case 'x':
      return benchmark.loopOverXyz();
    case 'z':
      return benchmark.loopOverZyx();
    case 'p':
      return benchmark.iterateOverPositions();
    case 'i':
      return benchmark.loopOverIndices();
    case 'v':
      return benchmark.iterateOverValues();
    case '+':
      return benchmark.callOperator();
    case 'g':
      return benchmark.callGenerate();
    default:
      throw std::runtime_error("Case not implemented"); // FIXME CaseNotImplemented
  }
}

class RasterBenchmarkIteration : public Elements::Program {

public:
  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override {
    Cnes::ProgramOptions options;
    options.named<char>("setup", "Test setup to be benchmarked (x, z, p, i, v)");
    options.named<long>("side", "Image width, height and depth (same value)", 400);
    return options.asPair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override {

    logger.info("Generating random rasters...");
    Cnes::IterationBenchmark benchmark(args["side"].as<long>());

    logger.info("Iterating over them...");
    const auto duration = iterate(benchmark, args["setup"].as<char>());

    logger.info() << "Done in " << duration.count() << "ms";

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(RasterBenchmarkIteration)
