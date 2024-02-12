// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Data/Raster.h"
#include "Linx/Io/Fits.h"
#include "Linx/Run/ProgramOptions.h"
#include "Linx/Run/Timer.h"
#include "LinxRun/Cosmics.h"

#include <boost/algorithm/string.hpp>
#include <map>
#include <string>
#include <vector>

int main(int argc, char const* argv[])
{
  Linx::ProgramOptions options;
  options.positional<std::string>("input", "The input data file name");
  options.positional<std::string>("output", "The output mask file name");
  options.named<std::string>("psf", "The PSF file name");
  options.named("hdu,i", "The 0-based input HDU index slice", 0L);
  options.named("pfa,p", "The detection probability of false alarm", 0.01);
  options.named("quotient,q", "The star rejection quotient threshold", 0.1);
  options.named("contrast,c", "The region-growing contrast threshold", 0.5);
  options.named("niter,n", "The number of segmentation iterations", 1L);
  options.parse(argc, argv);
  Linx::Fits data_fits(options.as<std::string>("input"));
  Linx::Fits map_fits(options.as<std::string>("output"));
  Linx::Fits psf_fits(options.as<std::string>("psf"));
  const auto hdu = options.as<Linx::Index>("hdu");
  const auto pfa = options.as<double>("pfa");
  const auto tq = options.as<double>("quotient");
  const auto tc = options.as<double>("contrast");
  const auto iter_count = options.as<Linx::Index>("niter");

  Linx::Timer<std::chrono::milliseconds> timer;

  std::cout << "Reading data: " << data_fits.path() << std::endl;
  auto data = data_fits.read<Linx::Raster<float>>(hdu);
  std::cout << "Reading PSF: " << psf_fits.path() << std::endl;
  auto psf = psf_fits.read<Linx::Raster<float>>();
  map_fits.write(data, 'w');

  std::cout << "Detecting cosmics..." << std::endl;
  timer.start();
  auto mask = Linx::Cosmics::detect(data, psf, pfa, tq);
  timer.stop();
  std::cout << "  Done in: " << timer.back().count() << " ms" << std::endl;
  std::cout << "  Density: " << Linx::mean(mask) << std::endl;
  map_fits.write(mask, 'a');

  std::cout << "Segmenting cosmics..." << std::endl;
  for (Linx::Index i = 0; i < iter_count; ++i) {
    std::cout << "  Iteration " << i + 1 << "/" << iter_count << std::endl;
    timer.start();
    Linx::Cosmics::segment(data, mask, tc);
    timer.stop();
    std::cout << "    Done in: " << timer.back().count() << " ms" << std::endl;
    std::cout << "    Density: " << Linx::mean(mask) << std::endl;
    map_fits.write(mask, 'a');
  }

  std::cout << "Saved map as: " << map_fits.path() << std::endl;

  return 0;
}
