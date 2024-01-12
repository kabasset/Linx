// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_DFTPLAN_H
#define _LINXTRANSFORMS_DFTPLAN_H

#include "Linx/Data/Raster.h"
#include "Linx/Transforms/DftMemory.h"

namespace Linx {

/**
 * @brief Memory- and computation-efficient discrete Fourier transform.
 * 
 * This class provides a light wrapping of FFTW's transforms.
 * It is design to compose transforms (e.g. direct and inverse DFTs) efficiently.
 * 
 * On memory side, one plan comes with an input buffer and an output buffer,
 * which are allocated at or prior to the plan construction.
 * Obviously, it is optimal to work directly in the buffers,
 * and avoid performing copies before and after transforms.
 * 
 * The most classical use case of composition
 * is to call the forward transform and later the inverse transform.
 * To this end, `inverse()` creates an inverse plan with shared buffers (see example below).
 * 
 * On computation side, the class relies on user-triggered evaluation
 * -- instead of early or lazy evaluations --,
 * i.e. the user has to explicitely call `transform()` when relevant.
 * The sequence is as follows:
 * - At construction, buffers are used to optimize the plan -- they contain garbage;
 * - The user fills the input buffer values;
 * - The user calls `transform()` -- now, the input buffer is garbage;
 * - The user reads the output buffer.
 * 
 * Here is a classical example to perform a convolution in Fourier domain:
 * 
 * \code
 * RealDft dft(shape);
 * auto idft = dft.inverse();
 * dft.in() = ... ; // Assign data somehow
 * dft.transform(); // Perform direct transform -- dft.in() = idft.out() is garbage now
 * const auto& coefficients = dft.out();
 * ... // Use coefficients, e.g. to convolve by a filter kernel
 * idft.transform().normalize(); // Perform inverse transform -- dft.out() = idft.in() is garbage now
 * const auto& filtered = idft.out();
 * ... // Do something with filtered, which contains the convolved signal
 * \endcode
 * 
 * Computation follows FFTW's conventions on formats and scaling.
 * If a buffer has Hermitian symmetry (e.g. the output of a direct real DFT),
 * the length along the first axis is `length0 / 2 + 1` if `length0` is the logical length along the first axis.
 * None of the transforms are scaled, which means that a factor is introduced
 * by calling `transform()` and then `inverse().transform()` -- `normalize()` performs normalization on request.
 * The factor equals the logical number of elements.
 * 
 * @tspecialization{ComplexDft}
 * @tspecialization{RealDft}
 * 
 * @see http://www.fftw.org/fftw3_doc
 */
template <typename TTransform, Index N = 2>
class DftPlan {
  template <typename UTransform, Index M>
  friend class DftPlan;

public:

  /**
   * @brief The plan dimension.
   */
  static constexpr Index Dimension = N;

  /**
   * @brief The kind of transform.
   */
  using Transform = TTransform;

  /**
   * @brief The kind of inverse transform.
   */
  using InverseTransform = typename Transform::InverseTransform;

  /**
   * @brief The inverse plan.
   */
  using Inverse = DftPlan<InverseTransform, N>;

  /**
   * @brief The input value type.
   */
  using InValue = typename Transform::InValue;

  /**
   * @brief The output value type.
   */
  using OutValue = typename Transform::OutValue;

  /**
   * @brief Constructor.
   * @param shape The logical shape
   * @param in_data The pre-existing input buffer, or `nullptr` to allocate a new one
   * @param out_data The pre-existing output buffer, or `nullptr` to allocate a new one
   */
  DftPlan(Position<N> shape, InValue* in_data = nullptr, OutValue* out_data = nullptr) :
      m_shape {shape}, m_in {Transform::in_shape(m_shape), in_data}, m_out {Transform::out_shape(m_shape), out_data},
      m_plan {FftwAllocator::create_plan<Transform>(m_in, m_out)}
  {}

  LINX_DEFAULT_COPYABLE(DftPlan)
  LINX_DEFAULT_MOVABLE(DftPlan)

  /**
   * @brief Destructor.
   * @warning
   * Buffers are freed if they were allocated by the plan constructor.
   * If data has to outlive the `DftPlan` object, buffers should be copied beforehand.
   */
  ~DftPlan()
  {
    FftwAllocator::destroy_plan(m_plan);
  }

  /**
   * @brief Create the inverse plan with shared buffers.
   * 
   * \code
   * auto idft = dft.inverse();
   * dft.transform(); // Fills dft.out() = idft.in()
   * idft.transform().normalize(); // Fills idft.out() = dft.in()
   * \endcode
   * @warning
   * This plan (`dft` from the snippet) is the owner of the buffers, which will be freed by its destructor,
   * which means that the buffers of the inverse plan (`idft`) has the same life cycle.
   */
  Inverse inverse()
  {
    return Inverse {m_shape, m_out.data(), m_in.data()};
  }

  /**
   * @brief Create a `DftPlan` which shares its input buffer with this `DftPlan`'s output buffer.
   * 
   * \code
   * auto plan_b = plan_a.template compose<ComplexDft>();
   * plan_a.transform(); // Fills plan_a.out() = plan_b.in()
   * plan_b.transform(); // Fills plan_b.out()
   * \endcode
   * @warning
   * This plan (`plan_a` from the snippet) is the owner of its output buffer, which will be freed by its destructor,
   * which means that the input buffer of the composed plan (`plan_b`) has the same life cycle.
   */
  template <typename TPlan>
  TPlan compose(const Position<N>& shape)
  {
    return {shape, m_out.data(), nullptr};
  }

  /**
   * @brief Get the logical plane shape.
   */
  const Position<N>& logical_shape() const
  {
    return m_shape;
  }

  /**
   * @brief Get the input buffer shape.
   */
  const Position<N>& in_shape() const
  {
    return m_in.shape();
  }

  /**
   * @brief Access the input buffer.
   * @warning
   * Contains garbage after `transform()` has been called.
   */
  const AlignedRaster<InValue, N>& in() const
  {
    return m_in;
  }

  /**
   * @copydoc in()const
   */
  AlignedRaster<InValue, N>& in()
  {
    return m_in;
  }

  /**
   * @brief Get the output buffer shape.
   */
  const Position<N>& out_shape() const
  {
    return m_out.shape();
  }

  /**
   * @brief Access the output buffer.
   */
  const AlignedRaster<OutValue, N>& out() const
  {
    return m_out;
  }

  /**
   * @copydoc out()const
   */
  AlignedRaster<OutValue, N>& out()
  {
    return m_out;
  }

  /**
   * @brief Get the normalization factor.
   */
  double normalization_factor() const
  {
    return shape_size(m_shape);
  }

  /**
   * @brief Compute the transform.
   */
  DftPlan& transform()
  {
    fftw_execute(*m_plan);
    return *this;
  }

  /**
   * @brief Divide by the output buffer by the normalization factor.
   */
  DftPlan& normalize()
  {
    const auto factor = 1. / normalization_factor();
    m_out *= factor;
    return *this;
  }

private:

  /**
   * @brief The logical shape.
   */
  Position<N> m_shape;

  /**
   * @brief The input stack.
   */
  AlignedRaster<InValue, N> m_in;

  /**
   * @brief The output stack.
   */
  AlignedRaster<OutValue, N> m_out;

  /**
   * @brief The transform plan.
   */
  Internal::FftwPlanPtr m_plan;
};

} // namespace Linx

#endif
