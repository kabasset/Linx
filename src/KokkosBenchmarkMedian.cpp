// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#include "Linx/Data/Image.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Transforms/RankFiltering.h"

#include <Kokkos_Core.hpp>
#include <Kokkos_Timer.hpp>

namespace Linx {
namespace Test {

template <typename TFootprint>
class MedianFilter : public SpatialFilterMixin<TFootprint, MedianFilter<TFootprint>> {
public:

  MedianFilter(TFootprint footprint) : SpatialFilterMixin<TFootprint, MedianFilter>(LINX_MOVE(footprint)) {}

  std::string label() const
  {
    return "MedianFilter";
  }

  template <typename TIn>
  class Apply : public ApplySpatialFilterMixin<TFootprint, TIn, Apply<TIn>> {
  public:

    using value_type = typename TIn::value_type;
    using element_type = std::remove_cvref_t<value_type>;

    Apply(TFootprint footprint, TIn in) :
        ApplySpatialFilterMixin<TFootprint, TIn, Apply>(LINX_MOVE(footprint), LINX_MOVE(in)),
        m_neighbors(this->m_offsets.size())
    {}

    KOKKOS_INLINE_FUNCTION auto reduce(const auto& neighbors) const
    {
      auto array = m_neighbors.array();
      std::copy(neighbors.begin(), neighbors.end(), array.data());
      return median(array);
    }

  private:

    ArrayPool<element_type> m_neighbors; // FIXME TSpace
  };
};

} // namespace Test
} // namespace Linx

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

int main(int argc, char const* argv[])
{
  Linx::ProgramContext context("", argc, argv);
  context.named("image", "Input length along each axis", 2048);
  context.named("kernel", "Kernel length along each axis", 5);
  context.flag("parity", "Enable parity tag");
  context.parse();
  const auto image_diameter = context.as<int>("image");
  const auto kernel_diameter = context.as<int>("kernel");
  const auto kernel_parity = context.as<bool>("parity");
  const auto output_diameter = image_diameter - kernel_diameter + 1;

  std::cout << "Generating input and kernel..." << std::endl;
  const auto image = Linx::Image<float, 2>("input", image_diameter, image_diameter);
  const auto kernel = Linx::Box(Linx::Position<2>(), Linx::Position<2>(Linx::Constant(kernel_diameter)));
  for_each(
      "init image",
      image.domain(),
      KOKKOS_LAMBDA(int i, int j) { image(i, j) = i + j; });
  Kokkos::fence();
  print_2d(image);
  std::cout << "kernel:" << std::endl;
  std::cout << "  " << kernel.extent(0) << " x " << kernel.extent(1) << std::endl;

  std::cout << "Filtering..." << std::endl;
  Kokkos::Timer timer;
  auto output = Linx::Image<float, 2>("output", output_diameter, output_diameter);
  if (kernel_parity) {
    Linx::median_filter_to(kernel, image, output); // Parity tag is inferred from kernel
  } else {
    (Linx::Test::MedianFilter(kernel)(image)).copy_to(output); // Unknown parity
  }
  Kokkos::fence();
  const auto elapsed = timer.seconds();

  std::cout << "  Done in " << elapsed << " s" << std::endl;
  print_2d(output);

  return 0;
}
