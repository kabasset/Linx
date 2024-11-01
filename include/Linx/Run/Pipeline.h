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

struct Stop {};

template <typename TContext, typename TState>
class Update {
public:

  using value_type = typename TState::value_type;
  using element_type = typename std::remove_cvref_t<value_type>;

  Update(TContext context, TState state) : m_context(LINX_MOVE(context)), m_state(LINX_MOVE(state))
  {
    m_context.log(label(m_state)); // FIXME label the transform
  }

  template <typename T>
  auto operator|(T&& step) &&
  {
    return Pipeline::Update(LINX_MOVE(m_context), LINX_FORWARD(step)(LINX_MOVE(m_state)));
  }

  template <typename T>
  auto operator|(Span<T> span) &&
  {
    auto label = compose_label("Sequence", m_state);
    auto state = Sequence<element_type, -1>(label, span.size()).copy_from(m_state); // FIXME make -1 the default
    // FIXME offset
    return Pipeline::Update(LINX_MOVE(m_context), LINX_MOVE(state));
  }

  template <Index N>
  auto operator|(Box<N> box) &&
  {
    auto label = compose_label("Image", m_state);
    auto state = Image<element_type, N>(label, box.shape()).copy_from(m_state);
    // FIXME offset
    return Pipeline::Update(LINX_MOVE(m_context), LINX_MOVE(state));
  }

  auto operator|(Stop) &&
  {
    return LINX_MOVE(m_state);
  }

private:

  TContext m_context;
  TState m_state;
};

template <typename TLogger, typename TState>
Update<Start<TLogger>, TState> operator|(Start<TLogger> context, TState state)
{
  return {LINX_MOVE(context), LINX_MOVE(state)};
}

} // namespace Pipeline
} // namespace Linx

#endif
