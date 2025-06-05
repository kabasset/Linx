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
#include "Linx/Transforms/Shift.h" // FIXME rm

void print_2d(const auto& image)
{
  auto name = image.label();
  auto width = image.shape()[0];
  auto height = image.shape()[1];

  const auto& on_host = Linx::on_host(image);

  auto filename = name + ".fits";
  Linx::Fits(filename, 'w').write(image);
}

namespace Linx { // FIXME to Math.h

struct Sqrt {
  KOKKOS_INLINE_FUNCTION auto operator()(const auto& e) const
  {
    return Kokkos::sqrt(e);
  }
};

} // namespace Linx

Linx::Box<2> strel(Linx::Index radius)
{
  return {{-radius, -radius}, {radius + 1, radius + 1}};
}

namespace Linx {

template <Index... Radii> // FIXME strong type
auto box_median_filter()
{
  static constexpr Index Size = (1 * ... * Radii);
  return Linx::MedianFilter<Size, Linx::Box<sizeof...(Radii)>>({{-Radii...}, {Radii + 1 ...}});
  // FIXME return Linx::MedianFilter<Linx::SBox<Radii...>>();
}

template <typename T>
struct IsOffset : std::false_type {};
template <typename T>
struct IsOffset<Shift<T>> : std::true_type {}; // FIXME rename as Offset?
// FIXME IsOffset<Patch<T, Box/Slice>> : std::true_type {};

template <typename T>
constexpr bool is_offset()
{
  return IsOffset<T>::value;
}

/**
 * @brief Align a contiguous-domain 1D data container along an axis, reshaping it into an ND image.
 * @tparam I The axis to align the array along
 * @tparam N The rank of the output image (-1 is not supported)
 * 
 * The input can be a sequence or a 1D image, possibly offset.
 * The output is an image or an offset image.
 */
template <Index I, Index N = I + 1, typename TIn>
auto along(const TIn& in)
{
  const auto& r = root(in);
  auto shape = Position<N>("shape").fill(1);
  shape[I] = r.size();
  Image<typename TIn::element_type, N> out(r.label(), shape);
  for (Index i = 0; i < in.ssize(); ++i) {
    auto p = Position<N>("p");
    p[I] = i;
    out[p] = r[i];
  }
  if constexpr (not is_offset<TIn>()) {
    return out;
  } else {
    auto offset = Position<N>("offset");
    offset[I] = in.domain().start(0);
    return Shift(out, offset);
  }
}

/**
 * @brief Create a 1D sampled Gaussian kernel.
 * 
 * Example 2D Gaussian filtering of an image with 0-padding:
 * 
 * ```cpp
 * auto sigma = 3.0;
 * auto kernel = Linx::sampled_gaussian_kernel(sigma, 3 * sigma);
 * auto filter = Linx::convolution_along<0, 1>(kernel);
 * auto out = filter.pad(0)(in);
 * ```
 */
template <typename T>
Shift<Sequence<T, -1>> sampled_gaussian_kernel(const T& sigma, Index radius)
{
  auto kernel = Shift(Sequence<T, -1>("gaussian kernel", 2 * radius + 1), -radius);
  const auto norm = std::numbers::inv_sqrtpi / sigma;
  const auto factor = -0.5 / (sigma * sigma);
  for_each("Gaussian kernel", kernel.domain(), KOKKOS_LAMBDA(int i) { return norm * std::exp(i * i * factor); });
  return kernel;
}

template <Index I, Index N = I + 1> // FIXME rm N, support non matching ranks in FilterMixin
auto convolution_along(auto&& kernel)
{
  return Convolution(along<I, N>(LINX_FORWARD(kernel)));
}

template <Index I, Index N = I + 1>
auto correlation_along(auto&& kernel)
{
  return Correlation(along<I, N>(on_host(LINX_FORWARD(kernel))));
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
    auto satpixels = Linx::Image<bool, 2>("satpixels", data.shape());
    auto median5 = Linx::box_median_filter<2, 2>().lazy(data);
    auto local_satlevel = satlevel; // Prevents capture of *this by KOKKOS_LAMBDA
    Linx::for_each(
        label(),
        median5.domain(),
        KOKKOS_LAMBDA(int i, int j) {
          if (data(i, j) >= local_satlevel) {
            satpixels(i, j) = (median5(i, j) > (local_satlevel / 10));
          }
        });
    auto grow_mask = +mask; // Copy the borders
    Linx::Dilation(strel(1)).transform(mask, grow_mask);
    // FIXME auto grow_mask = Dilation::with_border_copy(mask)?
    Linx::Dilation(strel(2)).transform(satpixels, mask);
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
std::tuple<TData, Linx::Image<bool, 2>> lacosmic(
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

  Linx::Flow("Scale to electrons", logger).append(data).apply(Linx::Add(sensor.pssl), Linx::Multiply(sensor.gain));
  Linx::Flow("Find saturated stars", logger).append(mask, data).run(FindSaturatedStars(sensor.satlevel));
  auto [backgroundlevel] = Linx::Flow("Compute background level", logger).append(data, mask).run(BackgroundLevel());

  auto crmask = Linx::Image<bool, 2>("crmask", data.shape());
  auto psfk = Linx::sampled_gaussian_kernel(psffwhm * 2 * std::sqrt(2 * std::log(2)), psfsize);

  for (Linx::Index i = 1; i <= niter; ++i) {
    auto label = "Iteration " + std::to_string(i) + " / " + std::to_string(niter);
    logger(label, "Start");

    auto [m5] = Linx::Flow("Compute m5", logger).append(data).run(Linx::box_median_filter<2, 2>());

    auto [noise] = Linx::Flow("Compute noise map", logger)
                       .append(m5.copy_as("noise")) // FIXME copy only used for cleantype = median
                       .apply(Linx::Max(T(0.00001)), Linx::Add(sensor.readnoise * sensor.readnoise), Linx::Sqrt());

    auto [sp] =
        Linx::Flow("Compute S'", logger)
            .append(data)
            .run(Linx::Upsample(2), Linx::separable_laplacian<0, 1>(-1.))
            .apply(Linx::Max(T(0)))
            .run(Linx::MeanFilter(Linx::Box<2>({0, 0}, {2, 2})), Linx::Downsample(2))
            .append(noise)
            .apply(Linx::Divide(), Linx::Divide(2))
            .prepend_run(Linx::box_median_filter<2, 2>())
            .apply(Linx::Subtract(), Linx::Negate());

    auto [f] =
        Linx::Flow("Compute fine structure", logger)
            .append(data)
            .run(Linx::convolution_along<0, 2>(psfk).pad(T()), Linx::convolution_along<1, 2>(psfk).pad(T()))
            // FIXME convolution_along<0, 1>(psfk).pad(0)
            .append(data, noise)
            .apply(FineStructure<T>());

    auto [cosmics] =
        Linx::Flow("Find candidate CRs", logger)
            .append(crmask, sp, f)
            .apply(FindCandidates(det.sigclip, det.objlim))
            .run(Linx::Dilation(strel(1)))
            .append(crmask, sp)
            .apply(FindNeighborCandidates(det.sigclip))
            .run(Linx::Dilation(strel(1)))
            .append(crmask, sp)
            .apply(FindNeighborCandidates(det.sigclip * det.sigfrac));

    auto numcr = Linx::sum(cosmics);
    if (numcr > 0) {
      Linx::Flow("Update crmask", logger).append(crmask, cosmics).apply(Linx::Or());
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

  auto data = Linx::Image<double, 2>("data", extent, extent).generate("random data", Linx::GaussianRng<double>(0, 1));
  auto mask = Linx::Image<bool, 2>("mask", extent, extent).generate("random mask", Linx::UniformRng<int>({0, 2}));
  // FIXME init psfk

  print_2d(data);
  print_2d(mask);

  auto [cleanarr, crmask] = lacosmic(data, mask, DetectionParams {}, SensorParams {}, niter);

  print_2d(cleanarr);
  print_2d(crmask);

  return 0;
}
