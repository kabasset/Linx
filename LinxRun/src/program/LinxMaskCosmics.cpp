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
    options.named("pfa,p", "The probability of false alarm", 0.01);
    options.named("sigma,s", "The sigma-clipping factor", 3.);
    return options.as_pair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override
  {
    Linx::Fits input(args["input"].as<std::string>());
    Linx::Fits output(args["output"].as<std::string>());
    const auto hdu = args["hdu"].as<Linx::Index>();
    const auto pfa = args["pfa"].as<double>();
    const auto factor = args["sigma"].as<double>();

    Linx::Chronometer<std::chrono::milliseconds> chrono;

    logger.info() << "Reading data: " << input.path();
    auto data = input.read<Linx::Raster<double>>(hdu); // FIXME flexible type
    output.write(data, 'w');

    logger.info() << "Detecting cosmics...";
    chrono.start();
    auto mask = Linx::Cosmics::detect(data, pfa);
    chrono.stop();
    logger.info() << "  Done in: " << chrono.last().count() << " ms";
    output.write(mask, 'a');

    logger.info() << "Segmenting cosmics...";
    chrono.start();
    Linx::Cosmics::segment(data, mask, factor);
    chrono.stop();
    logger.info() << "  Done in: " << chrono.last().count() << " ms";
    logger.info() << "  Density: " << Linx::mean(mask);
    output.write(mask, 'a');

    logger.info() << "Saved output as: " << output.path();

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(LinxMaskCosmics)
