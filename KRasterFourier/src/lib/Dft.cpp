// Copyright (C) 2022, CNES
// This file is part of KRaster <github.com/kabasset/KRaster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "KRasterFourier/Dft.h"

#include "KRasterFourier/DftMemory.h"

namespace Kast {

template <>
Position<2> RealDftType::Parent::outShape(const Position<2>& shape) {
  return {shape[0] / 2 + 1, shape[1]};
}

template <>
Internal::FftwPlanPtr
Internal::allocateFftwPlan<RealDftType>(DftBuffer<double>& in, DftBuffer<std::complex<double>>& out) {
  const auto& shape = in.shape();
  return std::make_unique<fftw_plan>(fftw_plan_dft_r2c_2d(
      shape[1],
      shape[0],
      reinterpret_cast<double*>(in.data()),
      reinterpret_cast<fftw_complex*>(out.data()),
      FFTW_MEASURE));
}

template <>
Internal::FftwPlanPtr
Internal::allocateFftwPlan<Inverse<RealDftType>>(DftBuffer<std::complex<double>>& in, DftBuffer<double>& out) {
  const auto& shape = out.shape();
  return std::make_unique<fftw_plan>(fftw_plan_dft_c2r_2d(
      shape[1],
      shape[0],
      reinterpret_cast<fftw_complex*>(in.data()),
      reinterpret_cast<double*>(out.data()),
      FFTW_MEASURE));
}

template <>
Internal::FftwPlanPtr
Internal::allocateFftwPlan<ComplexDftType>(DftBuffer<std::complex<double>>& in, DftBuffer<std::complex<double>>& out) {
  const auto& shape = in.shape();
  return std::make_unique<fftw_plan>(fftw_plan_dft_2d(
      shape[0],
      shape[1],
      reinterpret_cast<fftw_complex*>(in.data()),
      reinterpret_cast<fftw_complex*>(out.data()),
      FFTW_FORWARD,
      FFTW_MEASURE));
}

template <>
Internal::FftwPlanPtr Internal::allocateFftwPlan<Inverse<ComplexDftType>>(
    DftBuffer<std::complex<double>>& in,
    DftBuffer<std::complex<double>>& out) {
  const auto& shape = out.shape();
  return std::make_unique<fftw_plan>(fftw_plan_dft_2d(
      shape[0],
      shape[1],
      reinterpret_cast<fftw_complex*>(in.data()),
      reinterpret_cast<fftw_complex*>(out.data()),
      FFTW_BACKWARD,
      FFTW_MEASURE));
}

template <>
Position<2> HermitianComplexDftType::Parent::inShape(const Position<2>& shape) {
  return {shape[0] / 2 + 1, shape[1]};
}

template <>
Position<2> HermitianComplexDftType::Parent::outShape(const Position<2>& shape) {
  return {shape[0] / 2 + 1, shape[1]};
}

template <>
Internal::FftwPlanPtr Internal::allocateFftwPlan<HermitianComplexDftType>(
    DftBuffer<std::complex<double>>& in,
    DftBuffer<std::complex<double>>& out) {
  return Internal::allocateFftwPlan<ComplexDftType>(in, out);
}

template <>
Internal::FftwPlanPtr Internal::allocateFftwPlan<Inverse<HermitianComplexDftType>>(
    DftBuffer<std::complex<double>>& in,
    DftBuffer<std::complex<double>>& out) {
  return Internal::allocateFftwPlan<Inverse<ComplexDftType>>(in, out);
}

} // namespace Kast