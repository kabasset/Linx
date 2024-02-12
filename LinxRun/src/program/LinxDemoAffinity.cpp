// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Data/Raster.h"
#include "Linx/Io/Fits.h"
#include "Linx/Run/ProgramOptions.h"
#include "Linx/Transforms/Affinity.h"
#include "Linx/Transforms/Extrapolation.h"
#include "Linx/Transforms/Interpolation.h"

#include <map>
#include <string>
#include <vector>

int main(int argc, char const* argv[])
{
  Linx::ProgramOptions options;
  options.positional<std::string>("output", "Output file name");
  options.named<Linx::Index>("side", "Image side", 128);
  options.named<float>("translate", "Translation along first axis", 0);
  options.named<float>("scale", "Scaling factor", 1);
  options.named<float>("rotate", "Rotation angle (deg)", 0);
  options.parse(argc, argv);
  const auto filename = options.as<std::string>("output");
  const auto side = options.as<Linx::Index>("side");
  const Linx::Index quarter = side / 4;
  const float vector = options.as<float>("translate");
  const float factor = options.as<float>("scale");
  const float angle = options.as<float>("rotate");

  Linx::Raster<float> input({side, side});
  input.fill(1);
  Linx::Box<2> square({quarter, quarter}, {3 * quarter, 3 * quarter});
  input(square).fill(2);
  auto extrapolator = Linx::extrapolation(input, 0.F);
  auto interpolator = Linx::interpolation<Linx::Linear>(extrapolator);

  Linx::Raster<float> output(input.shape());
  Linx::Affinity<2> affinity({2. * quarter, 2. * quarter});
  affinity += {vector, 0};
  affinity *= factor;
  affinity.rotate_deg(angle);

  affinity.transform(interpolator, output);

  Linx::Fits fits(filename);
  fits.write(output);
  return 0;
}
