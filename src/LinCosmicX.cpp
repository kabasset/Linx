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

void print_2d(const auto& image)
{
  auto name = image.label();
  auto width = image.shape()[0];
  auto height = image.shape()[1];
  std::cout << name << ":" << std::endl;
  std::cout << "  " << width << " x " << height << std::endl;

  const auto& on_host = Linx::on_host(image);
  std::cout << "  [" << on_host(0, 0) << ", ... , " << on_host(width - 1, height - 1) << "]" << std::endl;
}

Linx::Box<2> strel(Linx::Index radius)
{
  return {{-radius, -radius}, {radius + 1, radius + 1}};
}

struct Updatemask {
  double satlevel;

  std::string label() const
  {
    return "update mask";
  }

  auto operator()(const auto& data, const auto& mask) const
  {
    auto satpixels = Linx::Image<bool, 2>(data.shape());
    auto median5 = Linx::MedianFilter(strel(2)).lazy(data);
    Linx::for_each(
        label(),
        median5.domain(),
        KOKKOS_LAMBDA(int i, int j) {
          if (data(i, j) >= satlevel) {
            satpixels(i, j) = (median5(i, j) > (satlevel / 10));
          }
        });
    auto grow_mask = mask.copy_as("grow_mask");
    Linx::Dilation(strel(1)).transform(mask, grow_mask);
    // FIXME auto grow_mask = Dilation::with_border_copy(mask)?
    auto grow_satpixels = +satpixels;
    Linx::Dilation(strel(2)).transform(satpixels, grow_satpixels);
    grow_mask *= grow_satpixels;
    return std::make_tuple(data, grow_mask);
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
  print_2d(indat);
  print_2d(inmask);

  Linx::TimerLogger logger;
  auto [mask, cleanarr] = P::Run("cleanarr", logger) // Startup
      | P::Input(indat) | P::Apply(Linx::Add(pssl), Linx::Multiply(gain)) // Scale input data
      | P::Input(inmask) | Updatemask(satlevel); // Update mask

  print_2d(mask);
  print_2d(cleanarr);

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
