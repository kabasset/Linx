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
    options.positional<std::string>("input", "The input data file name");
    options.positional<std::string>("output", "The output mask file name");
    options.named<std::string>("psf", "The PSF file name");
    options.named("hdu,i", "The 0-based input HDU index slice", 0L);
    options.named("pfa,p", "The detection probability of false alarm", 0.01);
    options.named("quotient,q", "The star rejection quotient threshold", 0.1);
    options.named("contrast,c", "The region-growing contrast threshold", 0.5);
    options.named("niter,n", "The number of segmentation iterations", 1L);
    return options.as_pair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override
  {
    Linx::Fits data_fits(args["input"].as<std::string>());
    Linx::Fits map_fits(args["output"].as<std::string>());
    Linx::Fits psf_fits(args["psf"].as<std::string>());
    const auto hdu = args["hdu"].as<Linx::Index>();
    const auto pfa = args["pfa"].as<double>();
    const auto tq = args["quotient"].as<double>();
    const auto tc = args["contrast"].as<double>();
    const auto iter_count = args["niter"].as<Linx::Index>();

    Linx::Chronometer<std::chrono::milliseconds> chrono;

    logger.info() << "Reading data: " << data_fits.path();
    auto data = data_fits.read<Linx::Raster<float>>(hdu);
    logger.info() << "Reading PSF: " << psf_fits.path();
    auto psf = psf_fits.read<Linx::Raster<float>>();
    map_fits.write(data, 'w');

    logger.info() << "Detecting cosmics...";
    chrono.start();
    auto mask = Linx::Cosmics::detect(data, psf, pfa, tq);
    chrono.stop();
    logger.info() << "  Done in: " << chrono.last().count() << " ms";
    logger.info() << "  Density: " << Linx::mean(mask);
    map_fits.write(mask, 'a');

    logger.info() << "Segmenting cosmics...";
    for (Linx::Index i = 0; i < iter_count; ++i) {
      logger.info() << "  Iteration " << i + 1 << "/" << iter_count;
      chrono.start();
      Linx::Cosmics::segment(data, mask, tc);
      chrono.stop();
      logger.info() << "    Done in: " << chrono.last().count() << " ms";
      logger.info() << "    Density: " << Linx::mean(mask);
      map_fits.write(mask, 'a');
    }

    logger.info() << "Saved map as: " << map_fits.path();

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(LinxMaskCosmics)
