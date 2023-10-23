// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "Linx/Data/Raster.h"
#include "Linx/Io/Fits.h"
#include "Linx/Run/Chronometer.h"
#include "Linx/Run/ProgramOptions.h"
#include "LinxRun/Cosmics.h"

#include <boost/algorithm/string.hpp>
#include <map>
#include <string>
#include <vector>

Elements::Logging logger = Elements::Logging::getLogger("LinxMaskCosmics");

class LinxMaskCosmics : public Elements::Program {
public:

  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override
  {
    Linx::ProgramOptions options;
    options.positional<std::string>("input", "Input data file name");
    options.positional<std::string>("output", "Output mask file name");
    options.named("hdu,i", "The 0-based input HDU index slice", 0L);
    options.named("pfa,p", "The detection probability of false alarm", 0.01);
    options.named("threshold,t", "The segmentation distance threshold", 0.5);
    options.named("niter,n", "The number of segmentation iterations", 1L);
    return options.as_pair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override
  {
    Linx::Fits input(args["input"].as<std::string>());
    Linx::Fits output(args["output"].as<std::string>());
    const auto hdu = args["hdu"].as<Linx::Index>();
    const auto pfa = args["pfa"].as<double>();
    const auto threshold = args["threshold"].as<double>();
    const auto iter_count = args["niter"].as<Linx::Index>();

    Linx::Chronometer<std::chrono::milliseconds> chrono;

    logger.info() << "Reading data: " << input.path();
    auto data = input.read<Linx::Raster<double>>(hdu); // FIXME flexible type
    output.write(data, 'w');

    logger.info() << "Detecting cosmics...";
    chrono.start();
    auto mask = Linx::Cosmics::detect(data, pfa);
    chrono.stop();
    logger.info() << "  Done in: " << chrono.last().count() << " ms";
    logger.info() << "  Density: " << Linx::mean(mask);
    output.write(mask, 'a');

    logger.info() << "Segmenting cosmics...";
    for (Linx::Index i = 0; i < iter_count; ++i) {
      logger.info() << "  Iteration " << i + 1 << "/" << iter_count;
      chrono.start();
      Linx::Cosmics::segment(data, mask, threshold);
      chrono.stop();
      logger.info() << "    Done in: " << chrono.last().count() << " ms";
      logger.info() << "    Density: " << Linx::mean(mask);
      output.write(mask, 'a');
    }

    logger.info() << "Saved output as: " << output.path();

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(LinxMaskCosmics)
