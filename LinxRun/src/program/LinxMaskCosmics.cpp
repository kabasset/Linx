// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "Linx/Data/Raster.h"
#include "Linx/Io/Fits.h"
#include "Linx/Run/Chronometer.h"
#include "Linx/Run/ProgramOptions.h"
#include "LinxRun/Cosmics.h"

#include <boost/math/special_functions/erf.hpp>
#include <map>
#include <string>
#include <vector>

Elements::Logging logger = Elements::Logging::getLogger("LinxDemoAffinity");

class LinxDemoAffinity : public Elements::Program {

public:
  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override {
    Linx::ProgramOptions options;
    options.positional<std::string>("input", "Input data file name");
    options.positional<std::string>("output", "Output mask file name");
    options.named("hdu,i", "The 0-based input HDU index", 0L);
    options.named("pfa,p", "The probability of false alarm", 0.01);
    options.named("strel", "The radius of the structuring element for morphological closing", 0L);
    return options.as_pair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override {

    Linx::Fits input(args["input"].as<std::string>());
    Linx::Fits output(args["output"].as<std::string>());
    const auto hdu = args["hdu"].as<long>();
    const auto pfa = args["pfa"].as<double>();
    const auto radius = args["strel"].as<long>();
    const auto factor = boost::math::erfc_inv(pfa * 2) * std::sqrt(2);

    Linx::Chronometer<std::chrono::milliseconds> chrono;

    logger.info() << "Reading data: " << input.path();
    const auto data = input.read<Linx::Raster<double>>(hdu);

    logger.info() << "Detecting cosmics...";
    logger.info() << "  \\Phi^{-1}(" << pfa << ") = " << factor;
    chrono.start();
    auto mask = Linx::flag_cosmics<unsigned char>(data, factor);
    chrono.stop();
    logger.info() << "  Done in: " << chrono.last().count() << " ms";

    if (radius > 0) {
      logger.info() << "Closing detection...";
      chrono.start();
      Linx::close_flag(mask, radius);
      chrono.stop();
      logger.info() << "  Done in: " << chrono.last().count() << " ms";
    }

    logger.info() << "Writing output: " << output.path();
    output.write(data, 'w');
    output.write(mask, 'a');
    logger.info() << "  Done.";

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(LinxDemoAffinity)
