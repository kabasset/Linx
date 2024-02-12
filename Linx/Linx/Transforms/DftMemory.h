// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXTRANSFORMS_DFTMEMORY_H
#define _LINXTRANSFORMS_DFTMEMORY_H

#include <complex>
#include <fftw3.h>
#include <memory>

namespace Linx {

/// @cond
namespace Internal {

/**
 * @brief RAII wrapper for `fftw_plan`.
 * 
 * `FftwPlan::get()` returns an `fftw_plan`.
 */
using FftwPlanPtr = std::unique_ptr<fftw_plan>;

} // namespace Internal
/// @endcond

/**
 * @brief Thread-safe singleton class to ensure proper FFTW memory management.
 * 
 * This is a Meyer's singleton.
 * The destructor, which is executed once (at the end of the program), calls `fftw_cleanup()`.
 */
class FftwAllocator {
private:

  /**
   * @brief Private constructor.
   */
  FftwAllocator() {}

  /**
   * @brief Destructor which frees.
   */
  ~FftwAllocator()
  {
    fftw_cleanup();
  }

  /**
   * @brief Get the instance.
   * 
   * Get the singleton if it exists already or instantiate it otherwise,
   * which triggers cleanup at destruction, i.e. when program ends.
   */
  static FftwAllocator& instantiate()
  {
    static FftwAllocator allocator;
    return allocator;
  }

public:

  /**
   * @brief Deleted copy constructor.
   */
  FftwAllocator(const FftwAllocator&) = delete;

  /**
   * @brief Deleted copy assignment operator.
   */
  FftwAllocator& operator=(const FftwAllocator&) = delete;

  /**
   * @brief Create a plan.
   * @warning
   * `in` and `out` are filled with garbage.
   */
  template <typename TTransform, typename TIn, typename TOut>
  static Internal::FftwPlanPtr create_plan(TIn& in, TOut& out)
  {
    instantiate();
    return TTransform::allocate_fftw_plan(in, out);
  }

  /**
   * @brief Destroy a plan.
   */
  static void destroy_plan(Internal::FftwPlanPtr& plan)
  {
    if (plan) {
      fftw_destroy_plan(*plan);
    }
  }
};

} // namespace Linx

#endif
