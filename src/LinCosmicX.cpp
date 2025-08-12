// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Base/Random.h"
#include "Linx/Data/Image.h"
#include "Linx/Io/Fits.h"
#include "Linx/Run/Flow.h"
#include "Linx/Run/Logging.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/LinearFiltering.h"
#include "Linx/Transforms/Morphology.h"
#include "Linx/Transforms/RankFiltering.h"
#include "Linx/Transforms/Resampling.h"

using namespace Linx::Literals;

void print_2d(const auto& image)
{
  auto name = image.label();
  auto width = image.extent(0);
  auto height = image.extent(1);

  const auto& on_host = Linx::on_host(image);

  auto filename = name + ".fits";
  Linx::Fits(filename, 'w').write(image);
}

namespace Linx {

template <typename TFootprint>
auto box_median_filter(const TFootprint& footprint)
{
  static constexpr auto Size = TFootprint().size();
  return Linx::MedianFilter<Size, TFootprint>(TFootprint()); // FIXME detect size in MedianFilter
}

} // namespace Linx

struct FindSaturatedStars {
  double satlevel;

  std::string label() const
  {
    return "FindSaturatedStars";
  }

  template <typename TData, typename TMask>
  const TMask& operator()(const TMask& mask, const TData& data) const
  {
    const auto domain = data.domain();
    auto satpixels = Linx::default_init<bool>("satpixels", domain);
    auto median5 = Linx::box_median_filter(Linx::cube<2_D, 2>()).lazy(data);
    Linx::for_each(
        label(),
        Linx::erode(domain, 2),
        KOKKOS_CLASS_LAMBDA(int i, int j) {
          if (data(i, j) >= satlevel) {
            satpixels(i, j) = (median5(i, j) > (satlevel / 10));
          }
        });
    auto grow_mask = +mask; // Copy the borders
    Linx::Dilation(Linx::cube<2_D, 1>()).transform(mask, Linx::Patch(grow_mask, Linx::erode(domain, 1)));
    Linx::Dilation(Linx::cube<2_D, 2>()).transform(satpixels, Linx::Patch(mask, Linx::erode(domain, 2)));
    mask &= grow_mask;
    return mask;
  }
};

struct BackgroundLevel {
  std::string label() const
  {
    return "BackgroundLevel";
  }

  float operator()(const auto& data, const auto& mask) const // FIXME double?
  {
    std::vector<float> gooddata; // FIXME double? // FIXME on device?
    const auto& data_on_host = Linx::on_host(data);
    const auto& mask_on_host = Linx::on_host(mask);
    Linx::for_each<Kokkos::Serial>(label(), mask.domain(), [&](int i, int j) {
      if (not mask_on_host(i, j)) {
        gooddata.push_back(data_on_host(i, j));
      }
    });
    return Linx::introselect_n(gooddata, gooddata.size() / 2);
  }
};

template <typename T>
struct FineStructure {
  KOKKOS_INLINE_FUNCTION auto operator()(auto f, auto m, auto n) const
  {
    return std::max<T>(0.01, (f - m) / n);
  }
};

struct FindCandidates {
  double sigclip;
  double objlim;
  KOKKOS_INLINE_FUNCTION auto operator()(auto m, auto sp, auto f) const
  {
    return (not m) && (sp > sigclip) && (sp / f > objlim);
  }
};

struct FindNeighborCandidates {
  double sigclip;
  KOKKOS_INLINE_FUNCTION auto operator()(auto c, auto m, auto sp) const
  {
    return c && (not m) && (sp > sigclip);
  }
};

struct DetectionParams {
  double sigclip = 4.5;
  double sigfrac = 0.3;
  double objlim = 5.0;
};

struct SensorParams {
  double gain = 1.0;
  double readnoise = 6.5;
  double satlevel = 65536.0;
  double pssl = 0.0;
};

template <typename TData, typename TMask>
auto lacosmic(
    const TData& data,
    const TMask& mask,
    const DetectionParams& det,
    const SensorParams& sensor,
    Linx::Index niter = 4,
    bool sepmed = true,
    const std::string& cleantype = "meanmask",
    const std::string& fsmode = "median",
    const std::string& psfmodel = "gauss",
    double psffwhm = 2.5,
    Linx::Index psfsize = 7,
    // Linx::Image<float, 2> psfk = Linx::Image<float, 2>("PSF", 1, 1).fill(1), // FIXME
    double psfbeta = 4.765,
    bool verbose = false)
{
  using T = typename TData::element_type;

  Linx::TimerLogger logger;
  logger("Lacosmic", "Start");

  Linx::Flow("Scale to electrons", logger).append(data).transform(Linx::Add(sensor.pssl), Linx::Multiply(sensor.gain));
  Linx::Flow("Find saturated stars", logger).append(mask, data).run(FindSaturatedStars(sensor.satlevel));
  auto [backgroundlevel] = Linx::Flow("Compute background level", logger).append(data, mask).run(BackgroundLevel());

  auto crmask = Linx::default_init<bool>("crmask", data.domain());
  auto psfk = Linx::sampled_gaussian_kernel(psffwhm * 2 * std::sqrt(2 * std::log(2)), psfsize);

  for (Linx::Index i = 1; i <= niter; ++i) {
    auto label = "Iteration " + std::to_string(i) + " / " + std::to_string(niter);
    logger(label, "Start");

    auto [m5] = Linx::Flow("Compute m5", logger).append(data).run(Linx::box_median_filter(Linx::cube<2_D, 2>()));

    auto [noise] = Linx::Flow("Compute noise map", logger)
                       .append(m5.copy_as("noise")) // FIXME copy only used for cleantype = median
                       .transform(Linx::Max(T(0.00001)), Linx::Add(sensor.readnoise * sensor.readnoise), Linx::Sqrt());

    auto [sp] =
        Linx::Flow("Compute S'", logger)
            .append(data)
            .run(Linx::Upsample(2), Linx::separable_laplacian<0, 1>(-1.))
            .transform(Linx::Max(T(0)))
            .run(Linx::MeanFilter(Linx::shape<2, 2>()), Linx::Downsample(2))
            .append(noise)
            .transform(Linx::Divide(), Linx::Divide(2))
            .prepend_run(Linx::box_median_filter(Linx::cube<2_D, 2>()))
            .transform(Linx::Subtract(), Linx::Negate());

    auto [f] =
        Linx::Flow("Compute fine structure", logger)
            .append(data)
            .run(Linx::convolution_along<0, 2>(psfk).pad(T()), Linx::convolution_along<1, 2>(psfk).pad(T()))
            // FIXME convolution_along<0, 1>(psfk).pad(0)
            .append(data, noise)
            .transform(FineStructure<T>());

    auto [cosmics] =
        Linx::Flow("Find candidate CRs", logger)
            .append(crmask, sp, f)
            .transform(FindCandidates(det.sigclip, det.objlim))
            .run(Linx::Dilation(Linx::cube<2_D, 1>()))
            .append(crmask, sp)
            .transform(FindNeighborCandidates(det.sigclip))
            .run(Linx::Dilation(Linx::cube<2_D, 1>()))
            .append(crmask, sp)
            .transform(FindNeighborCandidates(det.sigclip * det.sigfrac));

    auto numcr = Linx::sum(cosmics);
    if (numcr > 0) {
      Linx::Flow("Update crmask", logger).append(crmask, cosmics).transform(Linx::Or());
      logger(label, std::to_string(numcr) + " cosmic pixels found");
    } else {
      logger(label, "No cosmic pixel found");
      // break; // FIXME
    }

    // FIXME Clean data
  }

  logger("Lacosmic", "Stop");
  return std::make_tuple(data, crmask);
}

int main(int argc, char const* argv[])
{
  Linx::ProgramContext context("", argc, argv);
  context.named("image", "Input length along each axis", 4000);
  context.named("niter", "Kernel length along each axis", 4);
  context.parse();
  const auto extent = context.as<Linx::Index>("image");
  const auto niter = context.as<Linx::Index>("niter");

  auto data = Linx::generate("random data", Linx::GaussianRng<double>(0, 1), extent, extent);
  auto mask = Linx::generate("random mask", Linx::UniformRng<bool>(), extent, extent);
  // FIXME init psfk

  print_2d(data);
  print_2d(mask);

  auto [cleanarr, crmask] = lacosmic(data, mask, DetectionParams {}, SensorParams {}, niter);

  print_2d(cleanarr);
  print_2d(crmask);

  return 0;
}
