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

template <typename TLogger>
class StartPipeline {
public:

  StartPipeline(const std::string& label, TLogger& logger = nullptr) : m_label(label), m_logger(logger)
  {
    log(std::string("Pipeline start: ") + label);
  }

  const std::string& label() const
  {
    return m_label;
  }

  void log(auto content)
  {
    m_logger << content;
  }

private:

  std::string m_label;
  TLogger& m_logger;
};

template <>
class StartPipeline<void> {
public:

  StartPipeline(const std::string& label) : m_label(label) {}

  const std::string& label() const
  {
    return m_label;
  }

  void log(auto) {}

private:

  std::string m_label;
};

struct StopPipeline {};

template <typename TContext, typename TState>
class Pipeline {
public:

  using value_type = typename TState::value_type;
  using element_type = typename std::remove_cvref_t<value_type>;

  Pipeline(TContext context, TState state) : m_context(LINX_MOVE(context)), m_state(LINX_MOVE(state))
  {
    m_context.log(label(m_state));
  }

  template <typename T>
  auto operator|(T&& step) &&
  {
    return Linx::Pipeline(LINX_MOVE(m_context), LINX_FORWARD(step)(LINX_MOVE(m_state)));
  }

  template <Index N>
  auto operator|(Box<N> box) &&
  {
    auto label = compose_label("FIXME", box.start(), box.stop());
    auto state = Image<element_type, N>(label, box.shape()).copy_from(m_state);
    // FIXME offset
    return Linx::Pipeline(LINX_MOVE(m_context), LINX_MOVE(state));
  }

  auto operator|(StopPipeline) &&
  {
    return LINX_MOVE(m_state);
  }

private:

  TContext m_context;
  TState m_state;
};

template <typename TLogger, typename TState>
Pipeline<StartPipeline<TLogger>, TState> operator|(StartPipeline<TLogger> context, TState state)
{
  return {LINX_MOVE(context), LINX_MOVE(state)};
}

} // namespace Linx

#endif
