// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "Linx/Data/Grid.h"
#include "Linx/Data/Mask.h"
#include "Linx/Data/Raster.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Run/Chronometer.h"
#include "Linx/Run/ProgramOptions.h"

#include <map>
#include <string>

Elements::Logging logger = Elements::Logging::getLogger("LinxBenchmarkRegions");

template <typename TDuration>
TDuration filter(Linx::Raster<int, 3>& in, const Linx::Box<3>& box, char setup)
{
  Linx::Chronometer<TDuration> chrono;
  //! [Make sparse regions]
  Linx::Grid<3> grid(box, Linx::Position<3>::one());
  Linx::Mask<3> mask(box);
  Linx::Sequence<Linx::Position<3>> sequence(box);
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

class LinxBenchmarkRegions : public Elements::Program {
public:

  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override
  {
    Linx::ProgramOptions options;
    options.named<char>(
        "case",
        "Initial of the test case to be benchmarked: "
        "b (box), g (grid), m (mask), s (sequence)");
    options.named<long>("side", "Image width, height and depth (same value)", 400);
    options.named<long>("radius", "Region radius", 10);
    return options.as_pair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override
  {
    using Duration = std::chrono::milliseconds;

    const auto setup = args["case"].as<char>();
    const auto side = args["side"].as<long>();
    const auto radius = args["radius"].as<long>();

    logger.info("Generating random raster...");
    auto raster = Linx::Raster<int, 3>({side, side, side});
    //! [Make box]
    const auto box = Linx::Box<3>::from_center(radius, {side / 2, side / 2, side / 2});
    //! [Make box]

    logger.info("Filtering it...");
    const auto duration = filter<Duration>(raster, box, setup);
    const auto count = std::accumulate(raster.begin(), raster.end(), 0);

    logger.info() << "Performed " << count << " additions";
    logger.info() << "in " << duration.count() << "ms";

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(LinxBenchmarkRegions)
