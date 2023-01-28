// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "LitlBase/AlignedBuffer.h"
#include "LitlRun/Chronometer.h"
#include "LitlRun/ProgramOptions.h"

#include <map>
#include <string>
#include <vector>

Elements::Logging logger = Elements::Logging::getLogger("LitlBenchmarkBuffers");

using Chrono = Litl::Chronometer<std::chrono::milliseconds>;

Chrono::Unit benchmarkBuffer(Litl::Index size, Litl::Index alignment) {
  Chrono chrono;
  logger.info("Assignment...");
  Litl::AlignedBuffer<long> buffer(size, nullptr, alignment);
  long sum = 0;
  logger.info("Iteration...");
  const auto end = buffer.data() + buffer.size();
  chrono.start();
  for (auto it = buffer.data(); it != end; ++it) {
    sum += *it;
  }
  const auto duration = chrono.stop();
  logger.info(std::to_string(sum));
  return duration;
}

Chrono::Unit benchmarkVector(Litl::Index size) {
  Chrono chrono;
  logger.info("Initialization...");
  std::vector<long> buffer(size);
  long sum = 0;
  logger.info("Iteration...");
  chrono.start();
  for (const auto& v : buffer) {
    sum += v;
  }
  const auto duration = chrono.stop();
  logger.info(std::to_string(sum));
  return duration;
}

class LitlBenchmarkIteration : public Elements::Program {

public:
  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override {
    Litl::ProgramOptions options;
    options.named<long>("align", "Alignment for an AlignedBuffer or 0 for a std::vector");
    options.named<long>("size", "Number of elements", 1000000);
    return options.asPair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override {

    const auto alignment = args["align"].as<long>();
    const auto size = args["size"].as<long>();

    if (alignment > 0) {
      const auto duration = benchmarkBuffer(size, alignment);
      logger.info() << "Done in " << duration.count() << "ms";
    } else {
      const auto duration = benchmarkVector(size);
      logger.info() << "Done in " << duration.count() << "ms";
      logger.info() << "Done in " << duration.count() << "ms";
    }

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(LitlBenchmarkIteration)
