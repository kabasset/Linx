// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Data/Image.h"
#include "Linx/Io/Fits.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/LinearFiltering.h"
#include "Linx/Transforms/Morphology.h"
#include "Linx/Transforms/RankFiltering.h"

void print_2d(const auto& image)
{
  auto width = image.extent(0);
  auto height = image.extent(1);

  const auto& on_host = Linx::on_host(image);
  for (int j = 0; j < height; ++j) {
    for (int i = 0; i < width; ++i) {
      std::cout << on_host(i, j) << " ";
    }
    std::cout << std::endl;
  }
}

auto filter(const auto& in, const std::string& name, const auto& radius)
{
  auto footprint = Linx::Box<2>({-radius, -radius}, {radius + 1, radius + 1});
  if (name == "median") {
    return Linx::MedianFilter(footprint)(in);
  } else if (name == "max") {
    return Linx::MaximumFilter(footprint)(in);
  } else if (name == "gaussian") {
    auto k = Linx::sampled_gaussian_kernel(radius, 3 * radius);
    return Linx::correlation_along<1>(k)(Linx::correlation_along<0>(k)(in)); // FIXME convolution_along<0, 1>
  } else if (name == "laplacian") {
    return Linx::separable_laplacian<0, 1>(1.)(in);
  } else {
    throw std::runtime_error(name); // TODO Linx::UnknownCase
  }
}

int main(int argc, char const* argv[])
{
  Linx::ProgramContext context("", argc, argv);
  context.named("image", "Image radius", 7);
  context.named("filter", "Type of filter", std::string("gaussian"));
  context.named("radius", "Kernel radius, if relevant", 1.);
  context.named("output", "Output file name", std::string("/tmp/impulse.fits"));
  context.parse();
  const auto in_radius = context.as<int>("image");
  const auto in_extent = 2 * in_radius + 1;
  const auto filter_name = context["filter"];
  const auto filter_radius = context.as<double>("radius");
  const auto output_name = context["output"];

  std::cout << "Generating input and kernel..." << std::endl;
  auto in = Linx::generate(
      "in",
      KOKKOS_LAMBDA(int i, int j) { return (i == in_radius && j == in_radius) ? 1. : 0.; },
      in_extent,
      in_extent);

  std::cout << "Filtering..." << std::endl;
  auto out = filter(in, filter_name, filter_radius);
  print_2d(out);

  Linx::Fits(output_name, 'w').write(out);
  std::cout << "Saved output to: " << output_name << std::endl;

  return 0;
}
