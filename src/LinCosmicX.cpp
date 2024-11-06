// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Base/Random.h"
#include "Linx/Data/Image.h"
#include "Linx/Run/Logging.h"
#include "Linx/Run/PipelineTasks.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/Correlation.h"
#include "Linx/Transforms/Morphology.h"
#include "Linx/Transforms/RankFiltering.h"

namespace P = Linx::Pipeline;

Linx::Box<2> strel(Linx::Index radius)
{
  return {{-radius, -radius}, {radius + 1, radius + 1}};
}

template <typename TMask>
struct Updatemask {
  const TMask& mask;
  double satlevel;

  std::string label() const
  {
    return "update mask";
  }

  auto operator()(const auto& data) const
  {
    auto satpixels = Linx::Image<bool, 2>(data.shape());
    auto median5 = Linx::SumFilter(strel(2))(data); // FIXME MedianFilter
    Linx::for_each(
        label(),
        median5.domain(),
        KOKKOS_LAMBDA(int i, int j) {
          if (data(i, j) >= satlevel) {
            satpixels(i, j) = (median5(i, j) > (satlevel / 10));
          }
        });
    auto grow_mask = +mask;
    Linx::SumFilter(strel(1))(mask).copy_to(grow_mask); // FIXME Dilation
    auto grow_satpixels = +satpixels;
    Linx::SumFilter(strel(2))(satpixels).copy_to(grow_satpixels); // FIXME Dilation
    grow_mask *= grow_satpixels;
    return grow_mask;
  }
};

auto lacosmicx(
    const auto& indat,
    const auto& inmask,
    double sigclip = 4.5,
    double sigfrac = 0.3,
    double objlim = 5.0,
    double gain = 1.0,
    double readnoise = 6.5,
    double satlevel = 65536.0,
    double pssl = 0.0,
    Linx::Index niter = 4,
    bool sepmed = true,
    const std::string& cleantype = "meanmask",
    const std::string& fsmode = "median",
    const std::string& psfmodel = "gauss",
    double psffwhm = 2.5,
    Linx::Index psfsize = 7,
    std::nullptr_t psfk = nullptr, // FIXME
    double psfbeta = 4.765,
    bool verbose = false)
{
  Linx::TimerLogger logger;
  auto [cleanarr] = P::Run("cleanarr", logger) | indat.copy_as("cleanarr") // Startup
      | P::Apply(Linx::Add(pssl), Linx::Multiply(gain)) | Updatemask(inmask, satlevel);
  return cleanarr; // FIXME
}

int main(int argc, char const* argv[])
{
  Linx::ProgramContext context("", argc, argv);
  context.named("image", "Input length along each axis", 2048);
  context.named("kernel", "Kernel length along each axis", 5);
  context.parse();
  const auto image_diameter = context.as<int>("image");
  const auto kernel_diameter = context.as<int>("kernel");

  auto indat = Linx::Image<double, 2>("data", image_diameter, image_diameter)
                   .generate("random noise", Linx::GaussianRng<double>(0, 1));
  auto inmask = Linx::Image<bool, 2>("mask", image_diameter, image_diameter)
                    .generate("random mask", Linx::UniformRng<int>({0, 2}));

  auto out = lacosmicx(indat, inmask);

  return 0;
}
