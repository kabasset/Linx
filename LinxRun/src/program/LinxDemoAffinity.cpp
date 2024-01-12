// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ElementsKernel/ProgramHeaders.h"
#include "Linx/Data/Raster.h"
#include "Linx/Io/Fits.h"
#include "Linx/Run/ProgramOptions.h"
#include "Linx/Transforms/Affinity.h"
#include "Linx/Transforms/Extrapolation.h"
#include "Linx/Transforms/Interpolation.h"

#include <map>
#include <string>
#include <vector>

Elements::Logging logger = Elements::Logging::getLogger("LinxDemoAffinity");

class LinxDemoAffinity : public Elements::Program {
public:

  std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override
  {
    Linx::ProgramOptions options;
    options.positional<std::string>("output", "Output file name");
    options.named<Linx::Index>("side", "Image side", 128);
    options.named<float>("translate", "Translation along first axis", 0);
    options.named<float>("scale", "Scaling factor", 1);
    options.named<float>("rotate", "Rotation angle (deg)", 0);
    return options.as_pair();
  }

  ExitCode mainMethod(std::map<std::string, VariableValue>& args) override
  {
    const auto filename = args["output"].as<std::string>();
    const auto side = args["side"].as<Linx::Index>();
    const Linx::Index quarter = side / 4;
    const float vector = args["translate"].as<float>();
    const float factor = args["scale"].as<float>();
    const float angle = args["rotate"].as<float>();

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

    return Elements::ExitCode::OK;
  }
};

MAIN_FOR(LinxDemoAffinity)
