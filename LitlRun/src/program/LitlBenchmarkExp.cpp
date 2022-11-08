// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "LitlCore/Raster.h"
#include "LitlRun/Chronometer.h"
#include "LitlRun/ProgramOptions.h"

#include <map>
#include <string>

Elements::Logging logger = Elements::Logging::getLogger("LitlBenchmarkExp");

void taylorExp(Litl::Raster<double>& inout, Litl::Index order) { // FIXME test
  auto exp = [=](double x) {
    double sum = 1 + x;
    double term = x;
    for (Litl::Index i = 2; i <= order; ++i) {
      term *= x / i;
      sum += term;
    }
    return sum;
  };
  inout.apply(exp);
}

class LitlBenchmarkExp : public Elements::Program {

public:
  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override {
    Litl::ProgramOptions options;
    options.named<long>("order", "Taylor series order (or -1 for std::exp)", -1);
    options.named<long>("side", "Image width and height (same value)", 4096);
    return options.asPair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override {

    const auto order = args["order"].as<long>();
    const auto side = args["side"].as<long>();

    using Duration = std::chrono::milliseconds;
    Litl::Chronometer<Duration> chrono;

    logger.info("Generating random raster...");
    auto raster = Litl::Raster<double>({side, side}).generate(Litl::GaussianNoise<double>(0, 1, 0));

    logger.info("Computing exponential...");
    chrono.start();
    if (order == -1) {
      raster.exp();
    } else {
      taylorExp(raster, order);
    }
    const auto duration = chrono.stop();

    logger.info() << "  found: " << raster;
    logger.info() << "  in " << duration.count() << "ms";

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(LitlBenchmarkExp)
