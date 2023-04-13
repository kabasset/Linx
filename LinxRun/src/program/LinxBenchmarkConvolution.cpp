// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "LinxRun/Chronometer.h"
#include "LinxRun/ProgramOptions.h"
#include "LinxTransforms/Kernel.h"

#include <map>
#include <string>

static Elements::Logging logger = Elements::Logging::getLogger("LinxBenchmarkConvolution");

using Image = Linx::Raster<float, 3>;
using Duration = std::chrono::milliseconds;

template <typename TDuration>
TDuration filter(Image& image, const Linx::Kernel<Linx::KernelOp::Convolution, float, 3>& kernel, char setup) {
  Linx::Chronometer<TDuration> chrono;
  chrono.start();
  switch (setup) {
    case 'd':
      image = kernel * extrapolate(image, 0.0F);
      break;
    case 'm': {
      auto tmp = image;
      // kernel.correlateMonolithTo(extrapolate(image, 0).patch(image.domain()), tmp); // FIXME refactor
      image = tmp;
    } break;
    default:
      throw std::runtime_error("Case not implemented"); // FIXME CaseNotImplemented
  }
  return chrono.stop();
}

class RasterBenchmarkCorrelation : public Elements::Program {

public:
  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override {
    Linx::ProgramOptions options;
    options.named("case", "Test case: d (default), m (monolith)", 'd');
    options.named("image", "Raster length along each axis", 400L);
    options.named("kernel", "Kernel length along each axis", 3L);
    return options.asPair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override {
    const auto setup = args["case"].as<char>();
    const auto image_diameter = args["image"].as<Linx::Index>();
    const auto kernel_diameter = args["kernel"].as<Linx::Index>();

    Linx::Position<3> image_shape {image_diameter, image_diameter, image_diameter};
    Linx::Position<3> kernel_shape {kernel_diameter, kernel_diameter, kernel_diameter};

    logger.info("Generating raster and kernel...");
    auto image = Image(image_shape).range();
    const auto kernel = Linx::convolution(Image(kernel_shape).range());
    logger.info() << "  input: " << image;

    logger.info("Filtering it...");
    const auto duration = filter<Duration>(image, kernel, setup);
    logger.info() << "  output: " << image;

    logger.info() << "Performed convolution in " << duration.count() << "ms";

    return ExitCode::OK;
  }
};

MAIN_FOR(RasterBenchmarkCorrelation)
