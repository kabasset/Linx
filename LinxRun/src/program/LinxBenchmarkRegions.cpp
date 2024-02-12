// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Data/Grid.h"
#include "Linx/Data/Mask.h"
#include "Linx/Data/Raster.h"
#include "Linx/Data/Sequence.h"
#include "Linx/Run/ProgramOptions.h"
#include "Linx/Run/Timer.h"

#include <map>
#include <string>

template <typename TDuration>
TDuration filter(Linx::Raster<int, 3>& in, const Linx::Box<3>& box, char setup)
{
  Linx::Timer<TDuration> timer;
  //! [Make sparse regions]
  Linx::Grid<3> grid(box, Linx::Position<3>::one());
  Linx::Mask<3> mask(box);
  Linx::Sequence<Linx::Position<3>> sequence(box);
  //! [Make sparse regions]
  timer.start();
  switch (setup) {
    case 'b':
      //! [Iterate over box]
      in(box) += 1;
      //! [Iterate over box]
      break;
    case 'g':
      //! [Iterate over grid]
      in(grid) += 1;
      //! [Iterate over grid]
      break;
    case 'm':
      //! [Iterate over mask]
      in(mask) += 1;
      //! [Iterate over mask]
      break;
    case 's':
      //! [Iterate over sequence]
      in(sequence) += 1;
      //! [Iterate over sequence]
      break;
    default:
      throw std::runtime_error("Case not implemented"); // FIXME CaseNotImplemented
  }
  return timer.stop();
}

int main(int argc, char const* argv[])
{
  using Duration = std::chrono::milliseconds;

  Linx::ProgramOptions options;
  options.named<char>(
      "case",
      "Initial of the test case to be benchmarked: "
      "b (box), g (grid), m (mask), s (sequence)");
  options.named("side", "Image width, height and depth (same value)", 400L);
  options.named("radius", "Region radius", 10L);
  options.parse(argc, argv);

  const auto setup = options.as<char>("case");
  const auto side = options.as<Linx::Index>("side");
  const auto radius = options.as<Linx::Index>("radius");

  std::cout << "Generating random raster..." << std::endl;
  auto raster = Linx::Raster<int, 3>({side, side, side});
  //! [Make box]
  const auto box = Linx::Box<3>::from_center(radius, {side / 2, side / 2, side / 2});
  //! [Make box]

  std::cout << "Filtering it..." << std::endl;
  const auto duration = filter<Duration>(raster, box, setup);
  const auto count = std::accumulate(raster.begin(), raster.end(), 0);

  std::cout << "  Performed " << count << " additions" << std::endl;
  std::cout << "  Done in " << duration.count() << "ms" << std::endl;

  return 0;
}