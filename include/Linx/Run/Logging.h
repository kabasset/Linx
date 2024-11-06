// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_RUN_LOGGING_H
#define LINX_RUN_LOGGING_H

#include "Linx/Run/Timer.h"

#include <ctime>
#include <iomanip> // put_time
#include <iostream>

namespace Linx {

/**
 * @brief A simplistic logger which prefixes messages with the GMT date-time.
 */
struct CerrLogger {
  constexpr void operator()(auto&&... args) const
  {
    auto t = std::time(nullptr);
    auto tm = *std::gmtime(&t);
    std::cerr << std::put_time(&tm, "%F %TZ");
    ((std::cerr << " | " << LINX_FORWARD(args)), ...) << std::endl;
  }
};

/**
 * @brief A logger which suffixes messages with the time elapsed between two messages.
 */
template <typename TLogger = CerrLogger> // FIXME add TUnit with GCC 12 support for unit ostream
class TimerLogger {
public:

  using Timer = Linx::Timer<std::chrono::milliseconds>;
  using Logger = TLogger;

  TimerLogger(Timer timer = Timer(), Logger logger = Logger()) : m_timer(timer), m_logger(LINX_MOVE(logger)) {}

  const Timer& timer() const
  {
    return m_timer;
  }

  Timer& timer()
  {
    return m_timer;
  }

  const TLogger& logger() const
  {
    return m_logger;
  }

  TLogger& logger()
  {
    return m_logger;
  }

  /**
   * @brief Call `start()` if not already running, or `split()` otherwise and print the split time.
   */
  void operator()(const std::string& label, auto&&... args)
  {
    if (m_timer.is_running()) {
      m_timer.split(label);
      m_logger(label, LINX_FORWARD(args)..., m_timer.back().count());
    } else {
      m_timer.start();
      m_logger(label, LINX_FORWARD(args)..., "");
    }
  }

private:

  Timer m_timer;
  TLogger m_logger;
};

} // namespace Linx

#endif
