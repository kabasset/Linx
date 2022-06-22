// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERFOURIER_DFT_H
#define _RASTERFOURIER_DFT_H

#include "RasterFourier/DftPlan.h"

#include <complex>

namespace Litl {

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
std::vector<int> fftwShape(const TRaster& raster) {
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
  static Position<N> inShape(const Position<N>& shape) {
    return shape;
  }

  /**
   * @brief Output buffer shape.
   * @param shape The logical shape
   */
  template <Index N>
  static Position<N> outShape(const Position<N>& shape) {
    return shape;
  }

  /**
   * @brief Allocate an FFTW plan.
   */
  template <Index N>
  static FftwPlanPtr allocateFftwPlan(AlignedRaster<TIn, N>& in, AlignedRaster<TOut, N>& out);
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
  static Position<N> inShape(const Position<N>& shape) {
    return DftTransformMixin<TIn, TOut, TTransform>::outShape(shape);
  }

  template <Index N>
  static Position<N> outShape(const Position<N>& shape) {
    return DftTransformMixin<TIn, TOut, TTransform>::inShape(shape);
  }

  template <Index N>
  static FftwPlanPtr allocateFftwPlan(AlignedRaster<TOut, N>& in, AlignedRaster<TIn, N>& out);
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
Position<N> RealDftTransform::Base::outShape(const Position<N>& shape) {
  auto out = shape;
  out[0] = out[0] / 2 + 1;
  return out;
}

template <>
template <Index N>
FftwPlanPtr RealDftTransform::Base::allocateFftwPlan(RealDftBuffer<N>& in, ComplexDftBuffer<N>& out) {
  auto shape = fftwShape(in);
  return std::make_unique<fftw_plan>(fftw_plan_dft_r2c(
      shape.size(),
      shape.data(),
      reinterpret_cast<double*>(in.data()),
      reinterpret_cast<fftw_complex*>(out.data()),
      FFTW_MEASURE));
}

template <>
template <Index N>
FftwPlanPtr Inverse<RealDftTransform>::Base::allocateFftwPlan(ComplexDftBuffer<N>& in, RealDftBuffer<N>& out) {
  auto shape = fftwShape(out);
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
FftwPlanPtr ComplexDftTransform::Base::allocateFftwPlan(ComplexDftBuffer<N>& in, ComplexDftBuffer<N>& out) {
  auto shape = fftwShape(in);
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
FftwPlanPtr Inverse<ComplexDftTransform>::Base::allocateFftwPlan(ComplexDftBuffer<N>& in, ComplexDftBuffer<N>& out) {
  auto shape = fftwShape(out);
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
 * @relates DftPlan
 * @brief Compute the complex DFT.
 */
template <typename TRaster>
ComplexDftBuffer<TRaster::Dimension> complexDft(const TRaster& in) {
  ComplexDft<TRaster::Dimension> plan(in.shape());
  std::copy(in.begin(), in.end(), plan.in().begin());
  plan.transform();
  return std::move(plan.out());
}

/**
 * @relates DftPlan
 * @brief Compute the inverse complex DFT.
 */
template <typename TRaster>
ComplexDftBuffer<TRaster::Dimension> inverseComplexDft(const TRaster& in) {
  typename ComplexDft<TRaster::Dimension>::Inverse plan(in.shape());
  std::copy(in.begin(), in.end(), plan.in().begin());
  plan.transform().normalize();
  return std::move(plan.out());
}

/**
 * @relates DftPlan
 * @brief Compute the real DFT.
 */
template <typename TRaster>
ComplexDftBuffer<TRaster::Dimension> realDft(const TRaster& in) {
  RealDft<TRaster::Dimension> plan(in.shape());
  std::copy(in.begin(), in.end(), plan.in().begin());
  plan.transform();
  return std::move(plan.out());
}

/**
 * @relates DftPlan
 * @brief Compute the inverse real DFT.
 */
template <typename TRaster>
RealDftBuffer<TRaster::Dimension> inverseRealDft(const TRaster& in, const Position<TRaster::Dimension>& shape) {
  typename RealDft<TRaster::Dimension>::Inverse plan(shape);
  std::copy(in.begin(), in.end(), plan.in().begin());
  plan.transform().normalize();
  return std::move(plan.out());
}

} // namespace Litl

#endif
