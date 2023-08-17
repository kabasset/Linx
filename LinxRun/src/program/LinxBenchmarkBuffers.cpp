// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "Linx/Base/AlignedBuffer.h"
#include "Linx/Run/Chronometer.h"
#include "Linx/Run/ProgramOptions.h"

#include <map>
#include <string>
#include <vector>

Elements::Logging logger = Elements::Logging::getLogger("LinxBenchmarkBuffers");

using Chrono = Linx::Chronometer<std::chrono::milliseconds>;

Chrono::Unit benchmark_buffer(Linx::Index size, Linx::Index alignment)
{
  Chrono chrono;
  logger.info("Assignment...");
  chrono.start();
  Linx::AlignedBuffer<long> buffer(size, nullptr, alignment);
  auto duration = chrono.stop();
  logger.info(std::to_string(duration.count()) + "ms");
  long sum = 0;
  logger.info("Iteration...");
  chrono.start();
  for (const auto& v : buffer) {
    sum += v + 1;
  }
  duration = chrono.stop();
  logger.info(std::to_string(sum));
  logger.info(std::to_string(duration.count()) + "ms");
  return chrono.elapsed();
}

Chrono::Unit benchmark_vector(Linx::Index size)
{
  Chrono chrono;
  logger.info("Initialization...");
  chrono.start();
  std::vector<long> buffer(size);
  auto duration = chrono.stop();
  logger.info(std::to_string(duration.count()) + "ms");
  long sum = 0;
  logger.info("Iteration...");
  chrono.start();
  for (const auto& v : buffer) {
    sum += v + 1;
  }
  duration = chrono.stop();
  logger.info(std::to_string(sum));
  logger.info(std::to_string(duration.count()) + "ms");
  return chrono.elapsed();
}

class LinxBenchmarkIteration : public Elements::Program {
public:

  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override
  {
    Linx::ProgramOptions options;
    options.named<long>("align", "Alignment for an AlignedBuffer or 0 for a std::vector");
    options.named<long>("size", "Number of elements", 1000000);
    return options.as_pair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override
  {
    const auto alignment = args["align"].as<long>();
    const auto size = args["size"].as<long>();

    if (alignment > 0) {
      const auto duration = benchmark_buffer(size, alignment);
      logger.info() << "Done in " << duration.count() << "ms";
    } else {
      const auto duration = benchmark_vector(size);
      logger.info() << "Done in " << duration.count() << "ms";
      logger.info() << "Done in " << duration.count() << "ms";
    }

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(LinxBenchmarkIteration)
