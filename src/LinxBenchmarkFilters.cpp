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

using namespace Linx::Literals;

auto filter(const auto& in, const auto& k, const std::string& name)
{
  if (name == "correlation") {
    return Linx::Correlation(k)(in);
  } else if (name == "median") {
    return Linx::MedianFilter(k.domain())(in);
  } else if (name == "median3") {
    return Linx::MedianFilter(Linx::cube<2_D, 1>())(in);
  } else if (name == "median5") {
    return Linx::MedianFilter(Linx::cube<2_D, 2>())(in);
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
  const auto in = Linx::no_init<T>("input", in_extent, in_extent).generate_offsets();
  const auto k = Linx::no_init<T>("kernel", k_extent, k_extent).generate_offsets();

  std::cout << Linx::PrintLimit(3);
  std::cout << in << std::endl;
  std::cout << k << std::endl;

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
