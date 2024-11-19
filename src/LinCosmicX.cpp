// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Base/Random.h"
#include "Linx/Data/Image.h"
#include "Linx/Run/Logging.h"
#include "Linx/Run/PipelineTasks.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/LinearFiltering.h"
#include "Linx/Transforms/Morphology.h"
#include "Linx/Transforms/RankFiltering.h"
#include "Linx/Transforms/Resampling.h"

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
    return "Updatemask";
  }

  template <typename TData, typename TMask>
  Linx::Image<bool, 2> operator()(const TData& data, const TMask& mask) const
  {
    auto satpixels = Linx::Image<bool, 2>("satpixels", data.shape());
    auto median5 = Linx::MedianFilter(strel(2)).lazy(data);
    auto local_satlevel = satlevel; // Prevents capture of *this by KOKKOS_LAMBDA
    Linx::for_each(
        label(),
        median5.domain(),
        KOKKOS_LAMBDA(int i, int j) {
          if (data(i, j) >= local_satlevel) {
            satpixels(i, j) = (median5(i, j) > (local_satlevel / 10));
          }
        });
    auto grow_mask = +mask;
    Linx::Dilation(strel(1)).transform(mask, grow_mask);
    // FIXME auto grow_mask = Dilation::with_border_copy(mask)?
    auto grow_satpixels = +satpixels;
    Linx::Dilation(strel(2)).transform(satpixels, grow_satpixels);
    grow_satpixels *= grow_mask;
    return grow_satpixels;
  }
};

struct Backgroundlevel {
  std::string label() const
  {
    return "Backgroundlevel";
  }

  float operator()(const auto& data, const auto& mask) const // FIXME double?
  {
    std::vector<float> gooddata; // FIXME double?
    Linx::for_each<Kokkos::Serial>(label(), mask.domain(), [&](int i, int j) {
      if (not mask(i, j)) {
        gooddata.push_back(data(i, j));
      }
    });
    return Linx::StdSort::nth(gooddata, gooddata.size() / 2);
  }
};

template <typename TData, typename TMask>
std::tuple<TData, Linx::Image<bool, 2>> lacosmicx(
    const TData& indata,
    const TMask& inmask,
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
    Linx::Image<float, 2> psfk = Linx::Image<float, 2>(), // FIXME
    double psfbeta = 4.765,
    bool verbose = false)
{
  Linx::TimerLogger logger;

  auto [cleanarr] = P::Run("Scale to electrons", logger) | indata | P::Generate(Linx::Add(pssl), Linx::Multiply(gain));
  auto [mask] = P::Run("Find saturated stars", logger) | P::Input(indata, inmask) | Updatemask(satlevel);
  auto [backgroundlevel] = P::Run("Get background level", logger) | P::Input(cleanarr, mask) | Backgroundlevel();

  const auto sigcliplow = sigfrac * sigclip;
  auto crmask = Linx::Image<bool, 2>("crmask", indata.shape());

  for (Linx::Index i = 1; i <= niter; ++i) {
    logger("Iteration " + std::to_string(i) + " / " + std::to_string(niter));

    auto [noise] = P::Run("Compute noise", logger) | cleanarr | Linx::MedianFilter(strel(2))
        | P::Generate([=](auto e) { return std::sqrt(std::max(e, 0.00001) + readnoise * readnoise); });

    // FIXME keep m5

    auto [s] = P::Run("Compute S", logger) | cleanarr
        | Linx::Upsample(2) /*| Linx::Laplacian<0, 1>(1)*/ | P::Apply(Linx::Max(0.)) // FIXME element_type(0)
        | Linx::SumFilter(Linx::Box<2>({0, 0}, {2, 2})) // FIXME Lazy SumFilter // FIXME MeanFilter
        | Linx::Downsample(2) | P::Input(noise) | P::Apply(Linx::Divide(), Linx::Divide(2));

    auto [sp] = P::Run("Compute S'", logger) | s | Linx::MedianFilter(strel(2)) | P::Input(s)
        | P::Apply(KOKKOS_LAMBDA(auto m_i, auto s_i) { return s_i - m_i; }); // FIXME negate?

    auto [f_tmp] = P::Run("Compute fine structure", logger) | cleanarr | Linx::Correlation(psfk); // FIXME avoid tmp?
    auto [f] = P::Run("Compute fine structure", logger) | f_tmp | Linx::MedianFilter(strel(3)) | P::Input(f_tmp, noise)
        | P::Apply(KOKKOS_LAMBDA(auto m_i, auto f_i, auto n_i) { return std::max(0.01, (f_i - m_i) / n_i); });

    auto [cosmics] = P::Run("Find candidate cosmic rays", logger) | P::Input(mask, sp, f)
        | P::Apply(KOKKOS_LAMBDA(auto m_i, auto sp_i, auto f_i) {
                       return (not m_i) && (sp_i > sigclip) && (sp_i / f_i > objlim);
                     })
        | Linx::Dilation(strel(1)) | P::Input(mask, sp)
        | P::Apply(KOKKOS_LAMBDA(auto c_i, auto m_i, auto sp_i) { return c_i && (not m_i) && (sp_i > sigclip); })
        | Linx::Dilation(strel(1)) | P::Input(mask, sp)
        | P::Apply(KOKKOS_LAMBDA(auto c_i, auto m_i, auto sp_i) { return c_i && (not m_i) && (sp_i) > sigcliplow; });

    auto numcr = Linx::sum(cosmics);
    logger(std::to_string(numcr) + " cosmic pixels found");
    if (numcr == 0) {
      break;
    }

    P::Run("Update crmask", logger) | P::Input(crmask, cosmics) | P::Apply(Linx::Or());

    // FIXME clean
  }

  return std::make_tuple(cleanarr, crmask);
}

int main(int argc, char const* argv[])
{
  Linx::ProgramContext context("", argc, argv);
  context.named("image", "Input length along each axis", 2048);
  context.named("kernel", "Kernel length along each axis", 5);
  context.parse();
  const auto image_diameter = context.as<int>("image");
  const auto kernel_diameter = context.as<int>("kernel");

  auto data = Linx::Image<double, 2>("data", image_diameter, image_diameter)
                  .generate("random noise", Linx::GaussianRng<double>(0, 1));
  auto mask = Linx::Image<bool, 2>("mask", image_diameter, image_diameter)
                  .generate("random mask", Linx::UniformRng<int>({0, 2}));

  print_2d(data);
  print_2d(mask);

  auto [cleanarr, crmask] = lacosmicx(data, mask);

  print_2d(cleanarr);
  print_2d(crmask);

  return 0;
}
