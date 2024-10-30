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

struct NoLogger {
  constexpr void operator<<(auto&&) const {}
};

struct CerrLogger {
  constexpr void operator<<(auto&& in) const
  {
    auto t = std::time(nullptr);
    auto tm = *std::gmtime(&t);
    std::cerr << std::put_time(&tm, "%F %T") << ": " << in << std::endl;
  }
};

struct NoTimer {
  static constexpr bool start()
  {
    return false;
  }

  static constexpr bool stop()
  {
    return false;
  }

  static constexpr bool split()
  {
    return false;
  }
};

template <typename TLogger = NoLogger, typename TTimer = NoTimer>
class PipelineContext {
public:

  PipelineContext(TLogger logger = TLogger(), TTimer timer = TTimer()) : m_logger(logger), m_timer(timer) {}

  void log(auto content)
  {
    m_logger << content;
  }

  void log_time()
  {
    auto time = m_timer.split();
    if (time) {
      log(time);
    }
  }

private:

  TLogger m_logger;
  TTimer m_timer;
};

struct Output {};

template <typename TContext, typename TView>
class Pipeline {
public:

  using value_type = typename TView::value_type;
  using element_type = typename std::remove_cvref_t<value_type>;

  Pipeline(TContext context, TView view) : m_context(context), m_view(view)
  {
    m_context.log(label(m_view));
    m_context.log_time();
  }

  template <typename T>
  auto operator|(T&& step) &&
  {
    return Linx::Pipeline(LINX_MOVE(m_context), LINX_FORWARD(step)(LINX_MOVE(m_view)));
  }

  template <Index N>
  auto operator|(Box<N> box) &&
  {
    auto label = compose_label("FIXME", box.start(), box.stop());
    auto view = Image<element_type, N>(label, box.shape()).copy_from(m_view);
    // FIXME offset
    return Linx::Pipeline(LINX_MOVE(m_context), LINX_MOVE(view));
  }

  auto operator|(Output) &&
  {
    return LINX_MOVE(m_view);
  }

private:

  TContext m_context;
  TView m_view;
};

template <typename TLogger, typename TTimer, typename TView>
Pipeline<PipelineContext<TLogger, TTimer>, TView> operator|(PipelineContext<TLogger, TTimer> context, TView view)
{
  return {LINX_MOVE(context), LINX_MOVE(view)};
}

} // namespace Linx

#endif
