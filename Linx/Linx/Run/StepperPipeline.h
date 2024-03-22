// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXRUN_STEPPERPIPELINE_H
#define _LINXRUN_STEPPERPIPELINE_H

#include "PipelineStep.h"

#include <chrono>
#include <map>
#include <numeric> // accumulate
#include <tuple>
#include <typeindex>

namespace Linx {

/**
 * @brief A pipeline or directed acyclic graph (DAG) which can be run step-by-step using lazy evaluation.
 * 
 * The main method, `get<S>()`, returns the value of step `S`.
 * If not already done, the prerequisites of `S` are first triggered, recursively.
 * Run times of the steps are stored; They are accessed with `milliseconds()`.
 * 
 * This class relies on the CRTP, i.e. child classes should inherit this class with their name as template parameter, e.g.
 * \code
 * class MyPipeline : public StepperPipeline<MyPipeline> {
 *   ...
 * };
 * \endcode
 * 
 * Child classes must provide a specialization of the following methods for each step `S`:
 * - `void evaluate_impl<S>()`, which evaluates `S` assuming upstream steps were already computed;
 * - `S::Value get_impl<S>()`, which returns the computed value of `S`.
 * 
 * A step `S` is a class which contains the following type definitions:
 * - `Value` is the return value type of `get<S>()`;
 * - `Prerequisite` is (are) the step(s) which must be run prior to `S`, or `void` if there is no prerequisite;
 *   Multiple prerequisites are describled with tuples.
 */
template <typename TDerived>
class StepperPipeline {
public:

  /**
   * @brief Evaluation of step `S`.
   */
  template <typename S>
  typename S::Value get()
  {
    if constexpr (S::Cardinality == 1) {
      get<typename S::Prerequisite>();
    } else if constexpr (S::Cardinality > 1) {
      get_multiple<typename S::Prerequisite>(std::make_index_sequence<S::Cardinality> {});
    }
    return evaluate_get<S>();
  }

  /**
   * @brief Check whether some step `S` has already been evaluated.
   */
  template <typename S>
  bool evaluated() const
  {
    return m_milliseconds.find(key<S>()) != m_milliseconds.end();
  }

  /**
   * @brief Get the elapsed time of step `S`.
   * @return The time in millisecond if the step was evaluated, or -1 otherwise.
   */
  template <typename S>
  double milliseconds() const
  {
    const auto it = m_milliseconds.find(key<S>());
    if (it != m_milliseconds.end()) {
      return it->second;
    }
    return -1;
  }

  /**
   * @brief Get the total elapsed time.
   */
  double milliseconds() const
  {
    return std::accumulate(m_milliseconds.begin(), m_milliseconds.end(), 0., [](const auto sum, const auto& e) {
      return sum + e.second;
    });
  }

protected:

  /**
   * @brief Reset to initial step.
   */
  void reset()
  {
    m_milliseconds.clear();
  }

private:

  /**
   * @brief Call `get()` on each element of a tuple.
   */
  template <typename STuple, std::size_t... Is>
  void get_multiple(std::index_sequence<Is...>)
  {
    using mock_unpack = int[];
    (void)mock_unpack {0, (get<std::tuple_element_t<Is, STuple>>(), void(), 0)...};
    // TODO could be done in threads!
  }

  /**
   * @brief Access to protected methods of `TDerived`.
   */
  template <typename S>
  struct Accessor : TDerived {
    /**
     * @brief Call `algo.evaluate_impl<S>()`.
     */
    static void evaluate(TDerived& algo)
    {
      auto f = &Accessor::template evaluate_impl<S>;
      (algo.*f)();
    }

    /**
     * @brief Call `algo.get_impl<S>()`.
     */
    static typename S::Value get(TDerived& algo)
    {
      auto f = &Accessor::template get_impl<S>;
      return (algo.*f)();
    }
  };

  /**
   * @brief Run step `S` if not done and return its output.
   */
  template <typename S>
  typename S::Value evaluate_get()
  {
    if (not evaluated<S>()) { // FIXME not thread-safe
      const auto start = std::chrono::high_resolution_clock::now();
      Accessor<S>::evaluate(derived());
      const auto stop = std::chrono::high_resolution_clock::now();
      m_milliseconds[key<S>()] = std::chrono::duration<double, std::milli>(stop - start).count();
    }
    return Accessor<S>::get(derived());
  }

  /**
   * @brief Cast as `TDerived`.
   */
  inline TDerived& derived()
  {
    return static_cast<TDerived&>(*this);
  }

private:

  /**
   * @brief Get the key of a step `S`.
   */
  template <typename S>
  std::type_index key() const
  {
    return std::type_index(typeid(S));
  }

  /**
   * @brief The set of performed steps and durations.
   */
  std::map<std::type_index, double> m_milliseconds;
};

} // namespace Linx

#endif
