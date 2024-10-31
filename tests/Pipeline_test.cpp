// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-License-Identifier: Apache-2.0

#define BOOST_TEST_MODULE PipelineTest

#include "Linx/Base/Functional.h"
#include "Linx/Base/Random.h"
#include "Linx/Data/Image.h"
#include "Linx/Run/Logging.h"
#include "Linx/Run/Pipeline.h"
#include "Linx/Run/ProgramContext.h"

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
  os << compose_label("Constant", c());
  return os;
}

auto generate(const std::string& label, const auto& func, std::integral auto... shape) // FIXME to Image
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

decltype(auto) compose(auto&& f)
{
  return LINX_FORWARD(f);
}

decltype(auto) compose(auto f0, auto... fs)
{
  return KOKKOS_LAMBDA(auto&&... args)
  {
    return compose(fs...)(f0(args...));
  };
}

template <typename... TFuncs>
struct Apply {
  Apply(TFuncs... fs) : m_func(compose(fs...)) {}

  std::string label() const
  {
    return "Apply";
  }

  decltype(auto) operator()(auto&& in0, auto&&... ins)
  {
    return in0.apply(label(), m_func, LINX_FORWARD(ins)...);
  }

  decltype(compose(std::declval<TFuncs>()...)) m_func;
};

} // namespace Linx

LINX_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(sequence_api_test)
{
  auto size = 10;
  auto logger = Linx::TimerLogger();
  auto out = Linx::StartPipeline("(1 + 2) * 3", logger) // init
      | Linx::Constant(1) | Linx::Slice(0, size) // input
      | Linx::Apply(Linx::Add(2), Linx::Multiply(3)) // pixelwise operations
      | Linx::StopPipeline(); // output
  std::cout << out << std::endl;
  for (const auto& kv : logger.timer()) {
    std::cout << kv.first << " - " << kv.second << "ms" << std::endl;
  }
  BOOST_TEST((out.ssize() == size));
  BOOST_TEST(out.contains_only(9));
}

BOOST_AUTO_TEST_CASE(image_api_test)
{
  auto width = 10;
  auto height = 3;
  auto logger = Linx::TimerLogger();
  auto out = Linx::StartPipeline("(1 + 2) * 3", logger) // init
      | Linx::Constant(1) | Linx::Box({0, 0}, {width, height}) // input
      | Linx::Apply(Linx::Add(2), Linx::Multiply(3)) // pixelwise operations
      | Linx::StopPipeline(); // output
  logger.logger() << "Done.";
  for (const auto& kv : logger.timer()) {
    std::cout << kv.first << " - " << kv.second << "ms" << std::endl;
  }
  BOOST_TEST((out.shape() == Linx::Position({width, height})));
  BOOST_TEST(out.contains_only(9));
}

BOOST_AUTO_TEST_SUITE_END()
