// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXTRANSFORMS_DFT_H
#define _LINXTRANSFORMS_DFT_H

#include "Linx/Transforms/DftPlan.h"

#include <complex>

namespace Linx {

/**
 * @ingroup dft
 * @brief DFT buffer of real data.
 */
template <Index N = 2>
using RealDftBuffer = AlignedRaster<double, N>;

/**
 * @ingroup dft
 * @brief DFT buffer of complex data.
 */
template <Index N = 2>
using ComplexDftBuffer = AlignedRaster<std::complex<double>, N>;

/// @cond
namespace Internal {

/**
 * @brief Convert a raster shape into an FFTW shape.
 */
template <typename TRaster>
std::vector<int> fftw_shape(const TRaster& raster) {
  std::vector<int> out(raster.shape().begin(), raster.shape().end());
  std::reverse(out.begin(), out.end());
  return out;
}

/**
 * @brief Inverse of a `DftTransformMixin`.
 */
template <typename TTransform>
struct Inverse; // Forward declaration for DftTransformMixin

/**
 * @brief Base DFT transform to be inherited.
 */
template <typename TIn, typename TOut, typename TDerived>
struct DftTransformMixin {

  /**
   * @brief The parent `DftTransformMixin` class.
   */
  using Base = DftTransformMixin;

  /**
   * @brief The concrete transform.
   */
  using Transform = TDerived;

  /**
   * @brief The input value type.
   */
  using InValue = TIn;

  /**
   * @brief The output value type.
   */
  using OutValue = TOut;

  /**
   * @brief The tag of the inverse transform type.
   */
  using InverseTransform = Inverse<Transform>;

  /**
   * @brief Input buffer shape.
   * @param shape The logical shape
   */
  template <Index N>
  static Position<N> in_shape(const Position<N>& shape) {
    return shape;
  }

  /**
   * @brief Output buffer shape.
   * @param shape The logical shape
   */
  template <Index N>
  static Position<N> out_shape(const Position<N>& shape) {
    return shape;
  }

  /**
   * @brief Allocate an FFTW plan.
   */
  template <Index N>
  static FftwPlanPtr allocate_fftw_plan(AlignedRaster<TIn, N>& in, AlignedRaster<TOut, N>& out);
};

/**
 * @brief Specialization for inverse transforms.
 */
template <typename TTransform, typename TIn, typename TOut>
struct DftTransformMixin<TIn, TOut, Inverse<TTransform>> {

  using Base = DftTransformMixin;
  using Transform = Inverse<TTransform>;
  using InValue = TOut;
  using OutValue = TIn;
  using InverseTransform = TTransform;

  template <Index N>
  static Position<N> in_shape(const Position<N>& shape) {
    return DftTransformMixin<TIn, TOut, TTransform>::out_shape(shape);
  }

  template <Index N>
  static Position<N> out_shape(const Position<N>& shape) {
    return DftTransformMixin<TIn, TOut, TTransform>::in_shape(shape);
  }

  template <Index N>
  static FftwPlanPtr allocate_fftw_plan(AlignedRaster<TOut, N>& in, AlignedRaster<TIn, N>& out);
};

template <typename TTransform>
struct Inverse : DftTransformMixin<typename TTransform::InValue, typename TTransform::OutValue, Inverse<TTransform>> {};

/**
 * @brief Real DFT type.
 */
struct RealDftTransform;
struct RealDftTransform : DftTransformMixin<double, std::complex<double>, RealDftTransform> {};

template <>
template <Index N>
Position<N> RealDftTransform::Base::out_shape(const Position<N>& shape) {
  auto out = shape;
  out[0] = out[0] / 2 + 1;
  return out;
}

template <>
template <Index N>
FftwPlanPtr RealDftTransform::Base::allocate_fftw_plan(RealDftBuffer<N>& in, ComplexDftBuffer<N>& out) {
  auto shape = fftw_shape(in);
  return std::make_unique<fftw_plan>(fftw_plan_dft_r2c(
      shape.size(),
      shape.data(),
      reinterpret_cast<double*>(in.data()),
      reinterpret_cast<fftw_complex*>(out.data()),
      FFTW_MEASURE));
}

template <>
template <Index N>
FftwPlanPtr Inverse<RealDftTransform>::Base::allocate_fftw_plan(ComplexDftBuffer<N>& in, RealDftBuffer<N>& out) {
  auto shape = fftw_shape(out);
  return std::make_unique<fftw_plan>(fftw_plan_dft_c2r(
      shape.size(),
      shape.data(),
      reinterpret_cast<fftw_complex*>(in.data()),
      reinterpret_cast<double*>(out.data()),
      FFTW_MEASURE));
}

/**
 * @brief Complex DFT type.
 */
struct ComplexDftTransform;
struct ComplexDftTransform : DftTransformMixin<std::complex<double>, std::complex<double>, ComplexDftTransform> {};

template <>
template <Index N>
FftwPlanPtr ComplexDftTransform::Base::allocate_fftw_plan(ComplexDftBuffer<N>& in, ComplexDftBuffer<N>& out) {
  auto shape = fftw_shape(in);
  return std::make_unique<fftw_plan>(fftw_plan_dft(
      shape.size(),
      shape.data(),
      reinterpret_cast<fftw_complex*>(in.data()),
      reinterpret_cast<fftw_complex*>(out.data()),
      FFTW_FORWARD,
      FFTW_MEASURE));
}

template <>
template <Index N>
FftwPlanPtr Inverse<ComplexDftTransform>::Base::allocate_fftw_plan(ComplexDftBuffer<N>& in, ComplexDftBuffer<N>& out) {
  auto shape = fftw_shape(out);
  return std::make_unique<fftw_plan>(fftw_plan_dft(
      shape.size(),
      shape.data(),
      reinterpret_cast<fftw_complex*>(in.data()),
      reinterpret_cast<fftw_complex*>(out.data()),
      FFTW_BACKWARD,
      FFTW_MEASURE));
}

} // namespace Internal
/// @endcond

/**
 * @ingroup dft
 * @brief Real DFT plan.
 */
template <Index N = 2>
using RealDft = DftPlan<Internal::RealDftTransform, N>;

/**
 * @ingroup dft
 * @brief Complex DFT plan.
 */
template <Index N = 2>
using ComplexDft = DftPlan<Internal::ComplexDftTransform, N>;

/**
 * @relatesalso DftPlan
 * @brief Compute the complex DFT.
 */
template <typename TRaster>
ComplexDftBuffer<TRaster::Dimension> complex_dft(const TRaster& in) {
  ComplexDft<TRaster::Dimension> plan(in.shape());
  std::copy(in.begin(), in.end(), plan.in().begin());
  plan.transform();
  return std::move(plan.out());
}

/**
 * @relatesalso DftPlan
 * @brief Compute the inverse complex DFT.
 */
template <typename TRaster>
ComplexDftBuffer<TRaster::Dimension> inverse_complex_dft(const TRaster& in) {
  typename ComplexDft<TRaster::Dimension>::Inverse plan(in.shape());
  std::copy(in.begin(), in.end(), plan.in().begin());
  plan.transform().normalize();
  return std::move(plan.out());
}

/**
 * @relatesalso DftPlan
 * @brief Compute the real DFT.
 */
template <typename TRaster>
ComplexDftBuffer<TRaster::Dimension> real_dft(const TRaster& in) {
  RealDft<TRaster::Dimension> plan(in.shape());
  std::copy(in.begin(), in.end(), plan.in().begin());
  plan.transform();
  return std::move(plan.out());
}

/**
 * @relatesalso DftPlan
 * @brief Compute the inverse real DFT.
 */
template <typename TRaster>
RealDftBuffer<TRaster::Dimension> inverse_real_dft(const TRaster& in, const Position<TRaster::Dimension>& shape) {
  typename RealDft<TRaster::Dimension>::Inverse plan(shape);
  std::copy(in.begin(), in.end(), plan.in().begin());
  plan.transform().normalize();
  return std::move(plan.out());
}

} // namespace Linx

#endif
