// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXRUN_STEPPERPIPELINE_H
#define _LINXRUN_STEPPERPIPELINE_H

#include <chrono>
#include <map>
#include <numeric> // accumulate
#include <tuple>
#include <typeindex>

namespace Linx {

/// @cond
namespace Internal {

/**
 * @relatesalso StepperPipeline
 * @brief Traits class which gives the cardinality (number of elements) of a type.
 * 
 * Cardinality of:
 * - `void` is 0;
 * - a tuple is its size;
 * - any other type is 1.
 */
template <typename T>
struct TypeCardinality {
  static constexpr std::size_t value = 1;
};

/**
 * @relatesalso StepperPipeline
 * @brief `void` specialization
 */
template <>
struct TypeCardinality<void> {
  static constexpr std::size_t value = 0;
};

/**
 * @relatesalso StepperPipeline
 * @brief Tuple specialization
 */
template <typename... Ts>
struct TypeCardinality<std::tuple<Ts...>> {
  static constexpr std::size_t value = sizeof...(Ts);
};

/**
 * @relatesalso StepperPipeline
 * @brief Cardinality of a step's prerequisite.
 * @see `TypeCardinality`
 */
template <typename S>
constexpr std::size_t prerequisite_cardinality()
{
  return TypeCardinality<typename S::Prerequisite>::value;
}

} // namespace Internal
/// @endcond

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
 * - `void evaluate_impl<S>()`, which evaluates `S` assuming upstream tasks were already computed;
 * - `S::Return get_impl<S>()`, which returns the computed value of `S`.
 * 
 * A step `S` is a class which contains the following type definitions:
 * - `Return` is the return value type of `get<S>()`;
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
  typename S::Return get()
  {
    if constexpr (Internal::prerequisite_cardinality<S>() == 1) {
      get<typename S::Prerequisite>();
    } else if constexpr (Internal::prerequisite_cardinality<S>() > 1) {
      get_multiple<typename S::Prerequisite>(std::make_index_sequence<Internal::prerequisite_cardinality<S>()> {});
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
    static typename S::Return get(TDerived& algo)
    {
      auto f = &Accessor::template get_impl<S>;
      return (algo.*f)();
    }
  };

  /**
   * @brief Run step `S` if not done and return its output.
   */
  template <typename S>
  typename S::Return evaluate_get()
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

/**
 * @brief Helper class to declare a pipeline step.
 * 
 * Usage:
 * \code
 * struct Step0 : PipelineStep<void, char> {};
 * struct Step1a : PipelineStep<Step0, short> {};
 * struct Step1b : PipelineStep<Step0, int> {};
 * struct Step2 : PipelineStep<std::tuple<Step1a, Step1b>, long>{};
 * \endcode
 */
template <typename TPrerequisite, typename TReturn>
struct PipelineStep {
  /**
   * @brief The step's prerequisite's.
   * 
   * Can be:
   * - `void` for no prerequisite;
   * - `std::tuple` for multiple prerequisites;
   * - Any other type for simple prerequisite.
   */
  using Prerequisite = TPrerequisite;

  /**
   * @brief The return type of the step.
   * 
   * This is exactly the return type of the `StepperPipeline::get()`
   * which can for example be a const reference.
   */
  using Return = TReturn;
};

} // namespace Linx

#endif
