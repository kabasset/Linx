// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Linx/Base/AlignedBuffer.h"
#include "Linx/Run/ProgramOptions.h"
#include "Linx/Run/Timer.h"

#include <map>
#include <string>
#include <vector>

using Timer = Linx::Timer<std::chrono::milliseconds>;

Timer::Unit benchmark_buffer(Linx::Index size, Linx::Index alignment)
{
  Timer timer;
  std::cout << "Assignment..." << std::endl;
  timer.start();
  Linx::AlignedBuffer<long> buffer(size, nullptr, alignment);
  auto duration = timer.stop();
  std::cout << "  Done in " << duration.count() << "ms" << std::endl; // FIXME cout << duration << endl?
  long sum = 0;
  std::cout << "Iteration..." << std::endl;
  timer.start();
  for (const auto& v : buffer) {
    sum += v + 1;
  }
  duration = timer.stop();
  std::cout << "  Sum:" << sum << std::endl;
  std::cout << "  Done in " << duration.count() << "ms" << std::endl;
  return timer.total();
}

Timer::Unit benchmark_vector(Linx::Index size)
{
  Timer timer;
  std::cout << "Initialization..." << std::endl;
  timer.start();
  std::vector<long> buffer(size);
  auto duration = timer.stop();
  std::cout << "  Done in " << duration.count() << "ms" << std::endl;
  long sum = 0;
  std::cout << "Iteration..." << std::endl;
  timer.start();
  for (const auto& v : buffer) {
    sum += v + 1;
  }
  duration = timer.stop();
  std::cout << "  Sum:" << sum << std::endl;
  std::cout << "  Done in " << duration.count() << "ms" << std::endl;
  return timer.total();
}

int main(int argc, char const* argv[])
{
  Linx::ProgramOptions options;
  options.named<long>("align", "Alignment for an AlignedBuffer or 0 for a std::vector");
  options.named<long>("size", "Number of elements", 1000000);
  options.parse(argc, argv);
  const auto alignment = options.as<long>("align");
  const auto size = options.as<long>("size");

  if (alignment > 0) {
    const auto duration = benchmark_buffer(size, alignment);
    std::cout << "Done in " << duration.count() << "ms" << std::endl;
  } else {
    const auto duration = benchmark_vector(size);
    std::cout << "Done in " << duration.count() << "ms" << std::endl;
  }

  return 0;
}
