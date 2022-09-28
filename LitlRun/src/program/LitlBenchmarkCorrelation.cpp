// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "LitlRun/Chronometer.h"
#include "LitlRun/ProgramOptions.h"
#include "LitlTransforms/Kernel.h"

#include <map>
#include <string>

static Elements::Logging logger = Elements::Logging::getLogger("LitlBenchmarkCorrelation");

template <typename TDuration>
TDuration filter(Litl::Raster<int, 3>& in, const Litl::Kernel<int, 3>& kernel, char setup) {
  Litl::Chronometer<TDuration> chrono;
  chrono.start();
  switch (setup) {
    case 'd':
      in = kernel * extrapolate(in, 0);
      break;
    case 'm': {
      auto tmp = in;
      kernel.correlateMonolithTo(extrapolate(in, 0).subraster(in.domain()), tmp);
      in = tmp;
    } break;
    default:
      throw std::runtime_error("Case not implemented"); // FIXME CaseNotImplemented
  }
  return chrono.stop();
}

class RasterBenchmarkCorrelation : public Elements::Program {

public:
  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override {
    Litl::ProgramOptions options;
    options.named("case", "Test case: d (default), m (monolith)", 'd');
    options.named("side", "Raster length along each axis", 100L);
    options.named("radius", "Kernel radius", 5L);
    options.named("seed", "Random seed", -1L);
    return options.asPair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override {
    const auto setup = args["case"].as<char>();
    const auto side = args["side"].as<Litl::Index>();
    const auto diameter = args["radius"].as<Litl::Index>() * 2 + 1;
    const auto seed = args["seed"].as<Litl::Index>();
    Litl::Raster<int, 3> in({side, side, side});
    const auto kernel = Litl::kernelize(
        Litl::Raster<int, 3>({diameter, diameter, diameter}).generate(Litl::UniformNoise<int>(0, 1, seed)));
    const auto out = kernel * extrapolate(in, 0);

    using Duration = std::chrono::milliseconds;

    logger.info("Generating random raster...");
    auto raster = Litl::Raster<int, 3>({side, side, side}).generate(Litl::GaussianNoise<int>(100, 15, seed));
    logger.info() << "input: " << raster;

    logger.info("Filtering it...");
    const auto duration = filter<Duration>(raster, kernel, setup);
    logger.info() << "output: " << raster;

    logger.info() << "Performed correlation";
    logger.info() << "in " << duration.count() << "ms";

    return ExitCode::OK;
  }
};

MAIN_FOR(RasterBenchmarkCorrelation)
