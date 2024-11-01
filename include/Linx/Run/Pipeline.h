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

  Start(const std::string& label, TLogger& logger = nullptr) : m_label(label), m_logger(logger)
  {
    log("Start pipeline");
  }

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
  TLogger& m_logger;
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
template <typename TContext, typename TTask, typename TState>
class Update {
public:

  using value_type = typename TState::value_type;
  using element_type = typename std::remove_cvref_t<value_type>;

  Update(TContext context, TTask task, TState state) : m_context(LINX_MOVE(context)), m_state(task(LINX_MOVE(state)))
  {
    if constexpr (std::is_same_v<TTask, Forward>) {
      m_context.log(label(state));
    } else {
      m_context.log(label(task));
    }
  }

  template <std::size_t I>
  const auto& get() const&
  {
    return m_state; // FIXME get<I>(m_states)
  }

  template <std::size_t I>
  auto& get() &
  {
    return m_state; // FIXME get<I>(m_states)
  }

  template <std::size_t I>
  const auto& get() const&&
  {
    return LINX_MOVE(m_state); // FIXME get<I>(m_states)
  }

  template <std::size_t I>
  auto get() &&
  {
    return LINX_MOVE(m_state); // FIXME get<I>(m_states)
  }

  auto operator|(auto&& task) &&
  {
    return Pipeline::Update(LINX_MOVE(m_context), LINX_FORWARD(task), LINX_MOVE(m_state));
  }

private:

  TContext m_context;
  std::remove_cvref_t<decltype(std::declval<TTask>()(std::declval<TState>()))> m_state;
};

template <typename TLogger, typename TState>
auto operator|(Start<TLogger> context, TState state)
{
  return Update(LINX_MOVE(context), Forward(), LINX_MOVE(state));
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
concept AnyUpdate = is_specialization<Update, T>;

template <typename T>
auto operator|(AnyUpdate auto&& pipeline, Span<T>&& span)
{
  return LINX_MOVE(pipeline) | RestrictSequence(LINX_FORWARD(span));
}

template <Index N>
auto operator|(AnyUpdate auto&& pipeline, Box<N>&& box)
{
  return LINX_MOVE(pipeline) | RestrictImage(LINX_FORWARD(box));
}

} // namespace Pipeline
} // namespace Linx

namespace std {

/**
 * @brief Enable structured bindings.
 */
template <typename TContext, typename TTask, typename TState>
struct tuple_size<Linx::Pipeline::Update<TContext, TTask, TState>> : std::integral_constant<std::size_t, 1> {};

/**
 * @brief Enable structured bindings.
 */
template <std::size_t I, typename TContext, typename TTask, typename TState>
struct tuple_element<I, Linx::Pipeline::Update<TContext, TTask, TState>> {
  using type = TState;
};

} // namespace std

#endif
