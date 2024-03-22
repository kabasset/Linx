// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXRUN_PIPELINESTEP_H
#define _LINXRUN_PIPELINESTEP_H

#include <tuple>

namespace Linx {

/**
 * @brief Helper class to declare a pipeline step.
 * @tparam T This is exactly the return type of `StepperPipeline::get()` which can for example be a const reference
 * @tparam TSteps The list of prerequisite steps, if any
 * 
 * Usage:
 * \code
 * struct Step0 : PipelineStep<char()> {};
 * struct Step1a : PipelineStep<float(Step0)> {};
 * struct Step1b : PipelineStep<int(Step0)> {};
 * struct Step2 : PipelineStep<bool(Step1a, Step1b)>{};
 * \endcode
 */
template <typename T, typename... TSteps>
struct PipelineStep;

/**
 * @brief A step without prerequisite.
 */
template <typename T>
struct PipelineStep<T()> {
  using Value = T;
  using Prerequisite = void;
  static constexpr std::size_t Cardinality = 0;
};

/**
 * @brief A step with a single prerequisite.
 */
template <typename T, typename TStep>
struct PipelineStep<T(TStep)> {
  using Value = T;
  using Prerequisite = TStep;
  static constexpr std::size_t Cardinality = 1;
};

/**
 * @brief A step with multiple prerequisites.
 */
template <typename T, typename... TSteps>
struct PipelineStep<T(TSteps...)> {
  using Value = T;
  using Prerequisite = std::tuple<TSteps...>;
  static constexpr std::size_t Cardinality = sizeof...(TSteps);
};

} // namespace Linx

#endif
