// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE PipelineTest

#include "Linx/Base/Functional.h"
#include "Linx/Base/Random.h"
#include "Linx/Data/Image.h"
#include "Linx/Run/Pipeline.h"
#include "Linx/Run/ProgramContext.h"

#include <boost/test/unit_test.hpp>

namespace Linx {

auto generate(const std::string& label, const auto& func, std::integral auto... shape)
{
  using T = std::remove_cvref_t<decltype(func())>;
  return Image<T, sizeof...(shape)>(label, shape...).generate("generate", func); // FIXME uninitialized
}

template <typename TArg, typename TFunc>
auto compose_functions(TArg&& arg, TFunc f)
{
  return LINX_MOVE(f)(LINX_FORWARD(arg));
}

template <typename TArg, typename TFunc0, typename... TFuncs>
auto compose_functions(TArg&& arg, TFunc0 f0, TFuncs... fs)
{
  return compose_functions(LINX_FORWARD(f0(arg)), LINX_MOVE(fs)...);
}

template <typename TFunc>
auto apply(TFunc func)
{
  return [=](const auto& in) {
    return in.apply("apply", func);
  };
}

template <typename TFunc0, typename... TFuncs>
auto apply(TFunc0 f0, TFuncs... fs)
{
  return [=](const auto& in) {
    return in.apply(
        "apply",
        KOKKOS_LAMBDA(const auto& e) { return compose_functions(e, f0, fs...); });
  };
}

} // namespace Linx

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(rng_test)
{
  auto width = 10;
  auto height = 3;
  auto in = Linx::generate("in", Linx::Constant(1), width, height);
  auto out = Linx::PipelineContext(Linx::CerrLogger()) | in // input
      | Linx::apply(Linx::Add(1), Linx::Multiply(3)) // pixelwise operations
      | Linx::Box({0, 0}, {width, height}); // output
  BOOST_TEST((out.shape() == in.shape()));
  BOOST_TEST(out.contains_only(6));
}

BOOST_AUTO_TEST_SUITE_END()
