// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_RUN_PIPELINETASKS_H
#define LINX_RUN_PIPELINETASKS_H

#include "Linx/Data/Patch.h"
#include "Linx/Run/Pipeline.h"

namespace Linx {
namespace Pipeline {

namespace Impl {

/**
 * @brief Task to restrict the domain of a sequence.
 * 
 * Prefer using the pipe operator on `domain` directly
 */
template <typename TDomain>
class RestrictSequence {
public:

  RestrictSequence(TDomain domain) : m_domain(LINX_MOVE(domain)) {}

  std::string label() const
  {
    return "Set domain";
  }

  template <typename TIn>
  auto operator()(const TIn& in) const
  {
    using T = std::remove_cvref_t<typename TIn::value_type>;
    return Sequence<T, -1>(label(), m_domain.size()).copy_from(in);
    // FIXME offset
  }

  template <typename... TIns>
  auto operator()(const TIns&... ins) const
  {
    return std::tuple(operator()(ins)...); // FIXME to State?
  }

private:

  TDomain m_domain;
};

/**
 * @brief Task to restrict the domain of an image.
 */
template <typename TDomain>
class RestrictImage {
public:

  RestrictImage(TDomain domain) : m_domain(LINX_MOVE(domain)) {}

  std::string label() const
  {
    return "Set domain";
  }

  template <typename TIn>
  auto operator()(const TIn& in) const
  {
    using T = std::remove_cvref_t<typename TIn::value_type>;
    return Image<T, TDomain::n>(label(), m_domain.shape()).copy_from(in);
    // FIXME offset
  }

private:

  TDomain m_domain;
};

} // namespace Impl

/**
 * @brief Set the pipeline domain.
 */
template <typename T>
auto operator|(AnyState auto&& pipeline, Span<T>&& span)
{
  return LINX_FORWARD(pipeline) | Impl::RestrictSequence(LINX_FORWARD(span));
}

/**
 * @brief Set the pipeline domain.
 */
template <Index N>
auto operator|(AnyState auto&& pipeline, Box<N>&& box)
{
  return LINX_FORWARD(pipeline) | Impl::RestrictImage(LINX_FORWARD(box));
}

/**
 * @brief Apply pointwise functions.
 * 
 * When the functor is fed with several funtions, they are applied in order,
 * i.e. conceptually `Apply(f, g, h)(args...)` performs `h(g(f(args...)))`.
 */
template <typename... TFuncs>
struct Apply {
  Apply(TFuncs... fs) : m_func(compose_functions(fs...)) {}

  std::string label() const
  {
    return "Apply pointwise";
  }

  decltype(auto) operator()(auto&& in0, auto&&... ins)
  {
    return in0.apply(label(), m_func, LINX_FORWARD(ins)...);
  }

  decltype(compose_functions(std::declval<TFuncs>()...)) m_func;
};

template <typename... TFuncs>
struct Generate {
  Generate(TFuncs... fs) : m_func(compose_functions(fs...)) {}

  std::string label() const
  {
    return "Generate pointwise";
  }

  auto operator()(auto&& in0, auto&&... ins)
  {
    return (+in0).apply(label(), m_func, LINX_FORWARD(ins)...);
  }

  decltype(compose_functions(std::declval<TFuncs>()...)) m_func;
};

} // namespace Pipeline
} // namespace Linx

#endif
