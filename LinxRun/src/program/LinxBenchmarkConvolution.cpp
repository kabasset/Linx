// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "Linx/Run/Chronometer.h"
#include "Linx/Run/ProgramOptions.h"
#include "Linx/Transforms/Kernel.h"

#include <map>
#include <string>

static Elements::Logging logger = Elements::Logging::getLogger("LinxBenchmarkConvolution");

using Image = Linx::Raster<float>;
using Kernel = const Linx::Kernel<Linx::KernelOp::Convolution, float>;
using Duration = std::chrono::milliseconds;

void filter_monolith(Image& image, Kernel& kernel)
{
  const auto extrapolation = Linx::extrapolate<Linx::NearestNeighbor>(image);
  const auto extrapolated = extrapolation.copy(image.domain() + kernel.window());
  // image = kernel.crop(extrapolated);
  auto patch = extrapolated.patch(kernel.window() - kernel.window().front());
  auto out_it = image.begin();
  for (const auto& p : image.domain()) {
    patch.translate(p);
    *out_it = kernel(patch);
    ++out_it;
    patch.translate_back(p);
  }
}

void filter_hardcoded(Image& image, Kernel& kernel)
{
  const auto extrapolation = Linx::extrapolate<Linx::NearestNeighbor>(image);
  const auto extrapolated = extrapolation.copy(image.domain() + kernel.window());
  const auto kernel_data = kernel.raster();
  auto it = kernel_data.end();
  Image out(image.shape());
  auto out_it = out.begin();
  const auto inner = image.domain() - kernel.window().front();
  auto patch = extrapolated.patch(inner);
  for (const auto& q : kernel.window()) {
    --it;
    patch.translate(q);
    const auto k = *it;
    for (const auto& v : patch) {
      *out_it += k * v;
      ++out_it;
    }
    patch.translate_back(q);
    out_it = out.begin();
  }
  image = out;
}

template <typename TDuration>
TDuration filter(Image& image, Kernel& kernel, char setup)
{
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
      filter_monolith(image, kernel);
      break;
    case 'h':
      filter_hardcoded(image, kernel);
      break;
    default:
      throw std::runtime_error("Case not implemented"); // FIXME CaseNotImplemented
  }
  return chrono.stop();
}

class RasterBenchmarkCorrelation : public Elements::Program {
public:

  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override
  {
    Linx::ProgramOptions options;
    options.named("case", "Test case: d (default), m (monolith)", 'd');
    options.named("image", "Raster length along each axis", 2048L);
    options.named("kernel", "Kernel length along each axis", 5L);
    return options.as_pair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override
  {
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
