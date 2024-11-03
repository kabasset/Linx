// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/KokkosTest
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_RUN_PIPELINE_H
#define LINX_RUN_PIPELINE_H

#include "Linx/Data/Patch.h"

#include <ctime>
#include <iomanip> // put_time
#include <iostream>

namespace Linx {
namespace Pipeline {

/**
 * @brief Pipeline starting context.
 */
template <typename TLogger>
class Start {
public:

  Start(const std::string& label, TLogger& logger = nullptr) : m_label(label), m_logger(logger) {}

  const std::string& label() const
  {
    return m_label;
  }

  void log(auto&&... args)
  {
    m_logger(m_label, LINX_FORWARD(args)...);
  }

private:

  std::string m_label;
  TLogger& m_logger; // FIXME enable owned logger
};

/**
 * @brief No-logger specialization.
 */
template <>
class Start<void> {
public:

  Start(const std::string& label) : m_label(label) {}

  const std::string& label() const
  {
    return m_label;
  }

  void log(auto&&...) {}

private:

  std::string m_label;
};

/**
 * @brief Pipeline update event.
 */
template <typename TContext, typename... TValues>
class State {
public:

  State(TContext context, const std::string& message, TValues... values) :
      State(LINX_MOVE(context), message, std::make_tuple(LINX_MOVE(values)...))
  {}

  State(TContext context, const std::string& message, std::tuple<TValues...> values) :
      m_context(LINX_MOVE(context)), m_values(LINX_MOVE(values))
  {
    m_context.log(message);
  }

  State(const State&) = default;
  State(State&&) = default;
  State& operator=(const State&) = default;
  State& operator=(State&&) = default;

  template <std::size_t I>
  const auto& get() const&
  {
    return std::get<I>(m_values);
  }

  template <std::size_t I>
  auto& get() &
  {
    return std::get<I>(m_values);
  }

  template <std::size_t I>
  const auto& get() const&&
  {
    if (not m_stopped) {
      m_context.log("Stop");
      m_stopped = true;
    }
    return LINX_MOVE(std::get<I>(m_values));
  }

  template <std::size_t I>
  auto get() &&
  {
    if (not m_stopped) {
      m_context.log("Stop");
      m_stopped = true;
    }
    return LINX_MOVE(std::get<I>(m_values));
  }

  template <typename TTask>
  auto operator|(TTask task) &&
  {
    auto task_label = label(task);
    auto out = eval(LINX_MOVE(task), LINX_MOVE(m_values), std::make_index_sequence<sizeof...(TValues)>());
    return Pipeline::State(LINX_MOVE(m_context), task_label, out);
  }

private:

  template <std::integral auto... Is, typename TTask>
  static decltype(auto) eval(TTask task, auto values, std::index_sequence<Is...>)
  {
    return LINX_MOVE(task)(std::get<Is>(values)...);
  }

  TContext m_context;
  std::tuple<TValues...> m_values;
  bool m_stopped = false;
};

template <typename TLogger, typename TValue>
auto operator|(Start<TLogger> context, TValue&& value)
{
  return State(LINX_MOVE(context), "Start", LINX_FORWARD(value));
}

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
    return Sequence<T, -1>(label(), m_domain.size()).copy_from(in); // FIXME make -1 the default
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

template <typename T>
concept AnyState = is_specialization<State, T>;

template <typename T>
auto operator|(AnyState auto&& pipeline, Span<T>&& span)
{
  return LINX_FORWARD(pipeline) | RestrictSequence(LINX_FORWARD(span));
}

template <Index N>
auto operator|(AnyState auto&& pipeline, Box<N>&& box)
{
  return LINX_FORWARD(pipeline) | RestrictImage(LINX_FORWARD(box));
}

} // namespace Pipeline
} // namespace Linx

namespace std {

/**
 * @brief Enable structured bindings.
 */
template <typename TContext, typename... TValues>
struct tuple_size<Linx::Pipeline::State<TContext, TValues...>> :
    std::integral_constant<std::size_t, sizeof...(TValues)> {};

/**
 * @brief Enable structured bindings.
 */
template <std::size_t I, typename TContext, typename... TValues>
struct tuple_element<I, Linx::Pipeline::State<TContext, TValues...>> {
  using type = std::tuple_element_t<I, std::tuple<TValues...>>;
};

} // namespace std

#endif
