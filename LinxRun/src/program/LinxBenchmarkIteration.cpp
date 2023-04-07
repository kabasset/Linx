// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "LinxRun/IterationBenchmark.h"
#include "LinxRun/ProgramOptions.h"

#include <map>
#include <string>

Elements::Logging logger = Elements::Logging::getLogger("LinxBenchmarkIteration");

Linx::IterationBenchmark::Duration iterate(Linx::IterationBenchmark& benchmark, char setup) {
  switch (setup) {
    case 'x':
      return benchmark.loopOverXyz();
    case 'z':
      return benchmark.loopOverZyx();
    case 'p':
      return benchmark.iterateOverPositions();
    case 'q':
      return benchmark.iterateOverPositionsOptimized();
    case 'i':
      return benchmark.loopOverIndices();
    case 'v':
      return benchmark.iterateOverValues();
    case 'o':
      return benchmark.callOperator();
    case 'g':
      return benchmark.callGenerate();
    default:
      throw std::runtime_error("Case not implemented"); // FIXME CaseNotImplemented
  }
}

class LinxBenchmarkIteration : public Elements::Program {

public:
  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override {
    Linx::ProgramOptions options;
    options.named<char>(
        "case",
        "Initial of the test case to be benchmarked: "
        "x (x-y-z), z (z-y-x), p (position), i (index), v (value), o (operator), g (generate)");
    options.named<long>("side", "Image width, height and depth (same value)", 400);
    return options.asPair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override {

    logger.info("Generating random rasters...");
    Linx::IterationBenchmark benchmark(args["side"].as<long>());

    logger.info("Iterating over them...");
    const auto duration = iterate(benchmark, args["case"].as<char>());

    logger.info() << "Done in " << duration.count() << "ms";

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(LinxBenchmarkIteration)