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

using Image = Linx::Raster<float>;
using Kernel = const Linx::Kernel<Linx::KernelOp::Convolution, float>;
using Duration = std::chrono::milliseconds;

void filterMonolith(Image& image, Kernel& kernel) {
  const auto extrapolation = Linx::extrapolate<Linx::NearestNeighbor>(image);
  const auto extrapolated = extrapolation.copy(image.domain() + kernel.window());
  // image = kernel.crop(extrapolated);
  auto patch = extrapolated.patch(kernel.window() - kernel.window().front());
  auto outIt = image.begin();
  for (const auto& p : image.domain()) {
    patch.translate(p);
    *outIt = kernel(patch);
    ++outIt;
    patch.translateBack(p);
  }
}

void filterHardcoded(Image& image, Kernel& kernel) {
  const auto extrapolation = Linx::extrapolate<Linx::NearestNeighbor>(image);
  const auto extrapolated = extrapolation.copy(image.domain() + kernel.window());
  const auto kernelData = kernel.raster();
  auto kIt = kernelData.end();
  Image out(image.shape());
  auto outIt = out.begin();
  const auto inner = image.domain() - kernel.window().front();
  auto patch = extrapolated.patch(inner);
  for (const auto& q : kernel.window()) {
    --kIt;
    patch.translate(q);
    const auto k = *kIt;
    for (const auto& v : patch) {
      *outIt += k * v;
      ++outIt;
    }
    patch.translateBack(q);
    outIt = out.begin();
  }
  image = out;
}

template <typename TDuration>
TDuration filter(Image& image, Kernel& kernel, char setup) {
  Linx::Chronometer<TDuration> chrono;
  chrono.start();
  switch (setup) {
    case '0':
      image = kernel * Linx::extrapolate(image, 0.0F);
      break;
    case 'd':
      image = kernel * Linx::extrapolate<Linx::NearestNeighbor>(image);
      break;
    case 'm':
      filterMonolith(image, kernel);
      break;
    case 'h':
      filterHardcoded(image, kernel);
      break;
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
    options.named("image", "Raster length along each axis", 2048L);
    options.named("kernel", "Kernel length along each axis", 5L);
    return options.asPair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override {
    const auto setup = args["case"].as<char>();
    const auto image_diameter = args["image"].as<Linx::Index>();
    const auto kernel_diameter = args["kernel"].as<Linx::Index>();

    Linx::Position<2> image_shape {image_diameter, image_diameter};
    Linx::Position<2> kernel_shape {kernel_diameter, kernel_diameter};

    logger.info("Generating raster and kernel...");
    auto image = Image(image_shape).range();
    const auto kernel = Linx::convolution(Image(kernel_shape).range());
    logger.info() << "  input: " << image;

    logger.info("Filtering...");
    const auto duration = filter<Duration>(image, kernel, setup);
    logger.info() << "  output: " << image;

    logger.info() << "  Done in " << duration.count() << "ms";

    return ExitCode::OK;
  }
};

MAIN_FOR(RasterBenchmarkCorrelation)
