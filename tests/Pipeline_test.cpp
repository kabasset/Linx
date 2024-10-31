// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE PipelineTest

#include "Linx/Base/Functional.h"
#include "Linx/Base/Random.h"
#include "Linx/Data/Image.h"
#include "Linx/Run/Pipeline.h"
#include "Linx/Run/ProgramContext.h"
#include "Linx/Run/Timer.h"

#include <boost/test/unit_test.hpp>

namespace Linx { // FIXME

template <typename T>
const auto& as_readonly(const Constant<T>& c)
{
  return c;
}

template <typename T>
auto& operator<<(std::ostream& os, const Constant<T>& c)
{
  os << c();
  return os;
}

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

BOOST_AUTO_TEST_CASE(api_test)
{
  auto width = 10;
  auto height = 3;
  auto logger = Linx::CerrLogger();
  auto timer = Linx::Timer<std::chrono::milliseconds, Linx::CerrLogger>({}, &logger);
  auto out = Linx::start("(1 + 1) * 3", timer) // init
      | Linx::Constant(1) | Linx::Box({0, 0}, {width, height}) // input
      | Linx::apply(Linx::Add(1), Linx::Multiply(3)) // pixelwise operations
      | Linx::Stop(); // output
  logger << "Done.";
  BOOST_TEST((out.shape() == Linx::Position({width, height})));
  BOOST_TEST(out.contains_only(6));
}

BOOST_AUTO_TEST_SUITE_END()
