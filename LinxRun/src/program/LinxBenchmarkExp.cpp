// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Data/Raster.h"
#include "Linx/Run/ProgramOptions.h"
#include "Linx/Run/Timer.h"

#include <map>
#include <string>

/**
 * Compute the exponential as a Tailor series.
 * @param inout The raster to be exponentialized in place
 * @param order The series order (must be >= 1)
 */
void taylor_exp(Linx::Raster<double>& inout, Linx::Index order)
{
  auto exp = [=](double x) {
    double sum = 1 + x;
    double term = x;
    for (Linx::Index i = 2; i <= order; ++i) {
      term *= x / i;
      sum += term;
    }
    return sum;
  };
  inout.apply(exp);
}

int main(int argc, char const* argv[])
{
  Linx::ProgramOptions options;
  options.named<long>("order", "Taylor series order (or -1 for std::exp)", -1);
  options.named<long>("side", "Image width and height (same value)", 4096);
  options.parse(argc, argv);
  const auto order = options.as<long>("order");
  const auto side = options.as<long>("side");

  using Duration = std::chrono::milliseconds;
  Linx::Timer<Duration> timer;

  std::cout << "Generating random raster..." << std::endl;
  auto raster = Linx::Raster<double>({side, side}).generate(Linx::GaussianNoise<double>(0, 1, 0));

  std::cout << "Computing exponential..." << std::endl;
  timer.start();
  switch (order) {
    case -1:
      raster.exp();
      break;
    case 0:
      raster.fill(1);
      break;
    case 1:
      raster += 1;
      break;
    default:
      taylor_exp(raster, order);
  }
  const auto duration = timer.stop();

  std::cout << "  found: " << raster << std::endl;
  std::cout << "  in " << duration.count() << "ms" << std::endl;

  return 0;
}
