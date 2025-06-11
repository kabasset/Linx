// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Data/Image.h"
#include "Linx/Run/Logging.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/LinearFiltering.h"
#include "Linx/Transforms/Morphology.h"
#include "Linx/Transforms/RankFiltering.h"

using T = std::int32_t;

void print_2d(const auto& image)
{
  auto name = image.label();
  auto width = image.extent(0);
  auto height = image.extent(1);
  std::cout << name << ":" << std::endl;
  std::cout << "  " << width << " x " << height << std::endl;

  const auto& on_host = Linx::on_host(image);
  std::cout << "  [" << on_host(0, 0) << ", " << on_host(1, 0) << ", ... , ";
  std::cout << on_host(width - 2, 0) << ", " << on_host(width - 1, 0) << "]" << std::endl;
  std::cout << "  [" << on_host(0, 1) << ", " << on_host(1, 1) << ", ... , ";
  std::cout << on_host(width - 2, 1) << ", " << on_host(width - 1, 1) << "]" << std::endl;
  std::cout << "  [" << on_host(0, height - 2) << ", " << on_host(1, height - 2) << ", ... , ";
  std::cout << on_host(width - 2, height - 2) << ", " << on_host(width - 1, height - 2) << "]" << std::endl;
  std::cout << "  [" << on_host(0, height - 1) << ", " << on_host(1, height - 1) << ", ... , ";
  std::cout << on_host(width - 2, height - 1) << ", " << on_host(width - 1, height - 1) << "]" << std::endl;
}

Linx::Image<T, 2> filter(const auto& in, const auto& k, const std::string& name)
{
  if (name == "correlation") {
    return Linx::Correlation(k)(in);
  } else if (name == "median") {
    return Linx::MedianFilter(k.domain())(in);
  } else if (name == "median3") {
    return Linx::MedianFilter<9, Linx::Box<2>>(k.domain())(in);
  } else if (name == "median5") {
    return Linx::MedianFilter<25, Linx::Box<2>>(k.domain())(in);
  } else if (name == "min") {
    return Linx::MinimumFilter(k.domain())(in);
  } else if (name == "laplacian") {
    return Linx::separable_laplacian<0, 1>()(in);
  } else {
    throw std::runtime_error(name); // FIXME Linx::UnknownCase
  }
}

int main(int argc, char const* argv[])
{
  Linx::ProgramContext context("", argc, argv);
  context.named("image", "Input length along each axis", 4000);
  context.named("kernel", "Kernel length along each axis", 15);
  context.named("filter", "Type of filter", std::string("correlation")); // FIXME const char* to string in named()
  context.named("iter", "Number of iterations", 10);
  context.parse();
  const auto in_extent = context.as<int>("image");
  const auto k_extent = context.as<int>("kernel");
  const auto filter_name = context.as<std::string>("filter");
  const auto iter_count = context.as<int>("iter");

  std::cout << "Generating input and kernel..." << std::endl;
  const auto in = Linx::Image<T, 2>("input", in_extent, in_extent).fill_with_distance_from_data();
  const auto k = Linx::Image<T, 2>("kernel", k_extent, k_extent).fill_with_distance_from_data();
  print_2d(in);
  print_2d(k);
  Kokkos::fence();

  Linx::TimerLogger logger;
  logger(filter_name, "Start");
  for (int i = 0; i < iter_count; ++i) {
    auto output = filter(in, k, filter_name);
    Kokkos::fence();
    logger(filter_name, "Iteration " + std::to_string(i + 1) + " / " + std::to_string(iter_count));
  }

  return 0;
}
