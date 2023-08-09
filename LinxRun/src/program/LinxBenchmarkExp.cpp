// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "Linx/Data/Raster.h"
#include "Linx/Run/Chronometer.h"
#include "Linx/Run/ProgramOptions.h"

#include <map>
#include <string>

Elements::Logging logger = Elements::Logging::getLogger("LinxBenchmarkExp");

/**
 * Compute the exponential as a Tailor series.
 * @param inout The raster to be exponentialized in place
 * @param order The series order (must be >= 1)
 */
void taylor_exp(Linx::Raster<double>& inout, Linx::Index order) {
  auto exp = [=](double x) {
    double sum = 1 + x;
    double term = x;
    for (Linx::Index i = 2; i <= order; ++i) {
      term *= x / i;
      sum += term;
    }
    return sum;
  };
  inout.apply(exp);
}

class LinxBenchmarkExp : public Elements::Program {

public:
  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override {
    Linx::ProgramOptions options;
    options.named<long>("order", "Taylor series order (or -1 for std::exp)", -1);
    options.named<long>("side", "Image width and height (same value)", 4096);
    return options.as_pair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override {

    const auto order = args["order"].as<long>();
    const auto side = args["side"].as<long>();

    using Duration = std::chrono::milliseconds;
    Linx::Chronometer<Duration> chrono;

    logger.info("Generating random raster...");
    auto raster = Linx::Raster<double>({side, side}).generate(Linx::GaussianNoise<double>(0, 1, 0));

    logger.info("Computing exponential...");
    chrono.start();
    switch (order) {
      case -1:
        raster.exp();
        break;
      case 0:
        raster.fill(1);
        break;
      case 1:
        raster += 1;
        break;
      default:
        taylor_exp(raster, order);
    }
    const auto duration = chrono.stop();

    logger.info() << "  found: " << raster;
    logger.info() << "  in " << duration.count() << "ms";

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(LinxBenchmarkExp)
