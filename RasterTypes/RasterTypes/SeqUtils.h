// Copyright (C) 2022, CNES
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTERTYPES_SEQUTILS_H
#define _RASTERTYPES_SEQUTILS_H

#include <complex> // FIXME from TypeUtils?
#include <tuple>

namespace Cnes {

#define CNES_RASTER_SUPPORTED_INTS \
  bool, unsigned char, char, signed char, unsigned short, signed short, unsigned int, signed int, unsigned long, \
      signed long, unsigned long long, signed long long

#define CNES_RASTER_SUPPORTED_FLOATS float, double, long double

#define CNES_RASTER_SUPPORTED_COMPLEXES std::complex<float>, std::complex<double>, std::complex<long double>

#define CNES_RASTER_SUPPORTED_TYPES \
  CNES_RASTER_SUPPORTED_INTS, CNES_RASTER_SUPPORTED_FLOATS, CNES_RASTER_SUPPORTED_COMPLEXES

using RasterSupportedTypesTuple = std::tuple<CNES_RASTER_SUPPORTED_TYPES>;

#define CNES_RASTER_TEST_CASE_TEMPLATE(name) BOOST_AUTO_TEST_CASE_TEMPLATE(name, T, RasterSupportedTypesTuple)

} // namespace Cnes

#endif // _RASTERTYPES_SEQUTILS_H
