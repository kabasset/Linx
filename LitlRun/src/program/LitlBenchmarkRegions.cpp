// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "LitlContainer/Sequence.h"
#include "LitlRaster/Grid.h"
#include "LitlRaster/Mask.h"
#include "LitlRaster/Raster.h"
#include "LitlRun/Chronometer.h"
#include "LitlRun/ProgramOptions.h"

#include <map>
#include <string>

Elements::Logging logger = Elements::Logging::getLogger("LitlBenchmarkRegions");

template <typename TDuration>
TDuration filter(Litl::Raster<int, 3>& in, const Litl::Box<3>& box, char setup) {
  Litl::Chronometer<TDuration> chrono;
  //! [Make sparse regions]
  Litl::Grid<3> grid(box, Litl::Position<3>::one());
  Litl::Mask<3> mask(box);
  Litl::Sequence<Litl::Position<3>> sequence(box);
  //! [Make sparse regions]
  chrono.start();
  switch (setup) {
    case 'b':
      //! [Iterate over box]
      in.patch(box) += 1;
      //! [Iterate over box]
      break;
    case 'g':
      //! [Iterate over grid]
      in.patch(grid) += 1;
      //! [Iterate over grid]
      break;
    case 'm':
      //! [Iterate over mask]
      in.patch(mask) += 1;
      //! [Iterate over mask]
      break;
    case 's':
      //! [Iterate over sequence]
      in.patch(sequence) += 1;
      //! [Iterate over sequence]
      break;
    default:
      throw std::runtime_error("Case not implemented"); // FIXME CaseNotImplemented
  }
  return chrono.stop();
}

class LitlBenchmarkRegions : public Elements::Program {

public:
  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override {
    Litl::ProgramOptions options;
    options.named<char>(
        "case",
        "Initial of the test case to be benchmarked: "
        "b (box), g (grid), m (mask), s (sequence)");
    options.named<long>("side", "Image width, height and depth (same value)", 400);
    options.named<long>("radius", "Region radius", 10);
    return options.asPair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override {

    using Duration = std::chrono::milliseconds;

    const auto setup = args["case"].as<char>();
    const auto side = args["side"].as<long>();
    const auto radius = args["radius"].as<long>();

    logger.info("Generating random raster...");
    auto raster = Litl::Raster<int, 3>({side, side, side});
    //! [Make box]
    const auto box = Litl::Box<3>::fromCenter(radius, {side / 2, side / 2, side / 2});
    //! [Make box]

    logger.info("Filtering it...");
    const auto duration = filter<Duration>(raster, box, setup);
    const auto count = std::accumulate(raster.begin(), raster.end(), 0);

    logger.info() << "Performed " << count << " additions";
    logger.info() << "in " << duration.count() << "ms";

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(LitlBenchmarkRegions)
