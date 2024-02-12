// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Run/ProgramOptions.h"
#include "LinxRun/IterationBenchmark.h"

#include <map>
#include <string>

Linx::IterationBenchmark::Duration iterate(Linx::IterationBenchmark& benchmark, char setup)
{
  switch (setup) {
    case 'x':
      return benchmark.loop_over_xyz();
    case 'z':
      return benchmark.loop_over_zyx();
    case 'p':
      return benchmark.iterate_over_positions();
    case 'q':
      return benchmark.iterate_over_positions_optimized();
    case 'i':
      return benchmark.loop_over_indices();
    case 'v':
      return benchmark.iterate_over_values();
    case 'o':
      return benchmark.call_operator();
    case 'g':
      return benchmark.call_generate();
    default:
      throw std::runtime_error("Case not implemented"); // FIXME CaseNotImplemented
  }
}

int main(int argc, char const* argv[])
{
  Linx::ProgramOptions options;
  options.named<char>(
      "case",
      "Initial of the test case to be benchmarked: "
      "x (x-y-z), z (z-y-x), p (position), i (index), v (value), o (operator), g (generate)");
  options.named<long>("side", "Image width, height and depth (same value)", 400);
  options.parse(argc, argv);

  std::cout << "Generating random rasters..." << std::endl;
  Linx::IterationBenchmark benchmark(options.as<Linx::Index>("side"));

  std::cout << "Iterating over them..." << std::endl;
  const auto duration = iterate(benchmark, options.as<char>("case"));

  std::cout << "Done in " << duration.count() << "ms" << std::endl;

  return 0;
}
