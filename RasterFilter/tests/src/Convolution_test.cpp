// Copyright (C) 2022, Antoine Basset
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RasterFilter/Convolution.h"

#include <boost/test/unit_test.hpp>

using namespace Litl;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Convolution_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(kernel_1d_init_test) {
  Kernel1d<int, OutOfBoundsCrop> kernel({0, 1, 2, 3, 4}, 1);
  BOOST_TEST(kernel.size() == 5);
  BOOST_TEST(kernel.backward() == 1);
  BOOST_TEST(kernel.forward() == 3);
  BOOST_TEST(*kernel.originData() == 1);
  int i = 0;
  for (auto it = kernel.begin(); it != kernel.end(); ++it, ++i) {
    BOOST_TEST(*it == i);
  }
}

BOOST_AUTO_TEST_CASE(standard_convolve1d_test) {
  const std::vector<int> inData {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
  const std::vector<float> kernelData {0.5, 1., 1.5};
  std::vector<double> outData(inData.size(), 0.);
  const std::vector<double> expected {4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 16};
  DataSamples<const int> in(inData.data(), inData.size(), {0, Limits<Index>::inf()});
  Kernel1d<float> kernel(kernelData, 1);
  DataSamples<double> out(outData.data(), outData.size(), {0, Limits<Index>::inf()});
  kernel.sparseCorrelate1dTo(in, out);
  for (std::size_t i = 0; i < expected.size(); ++i) {
    BOOST_TEST(outData[i] == expected[i]);
  }
}

BOOST_AUTO_TEST_CASE(steped_convolve1d_test) {
  const std::vector<int> inData {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
  const std::vector<float> kernelData {0.5, 1., 1.5, 2., 1.};
  std::vector<double> outData {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  const std::vector<double> expected {1, 2, 8.5, 4, 26, 6, 44, 8, 62, 10, 65, 12};
  DataSamples<const int> in(inData.data(), inData.size(), {1, Limits<Index>::inf(), 3});
  Kernel1d<float> kernel(kernelData, 3);
  DataSamples<double> out(outData.data(), outData.size(), {2, Limits<Index>::inf(), 2});
  kernel.sparseCorrelate1dTo(in, out);
  for (std::size_t i = 0; i < expected.size(); ++i) {
    BOOST_TEST(outData[i] == expected[i]);
  }
}

template <typename T>
std::vector<T> transposePad(const std::vector<T>& in, Index stride) {
  std::vector<T> out(in.size() * stride, T());
  for (std::size_t i = 0; i < in.size(); ++i) {
    out[i * stride] = in[i];
  }
  return out;
}

BOOST_AUTO_TEST_CASE(transpose_pad_test) {
  const std::vector<int> in {1, 2, 3, 4};
  const auto out = transposePad(in, 3);
  const std::vector<int> expected {1, 0, 0, 2, 0, 0, 3, 0, 0, 4, 0, 0};
  BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(strided_convolve1d_test) {
  const auto inData = transposePad<int>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}, 2);
  const std::vector<float> kernelData {0.5, 1., 1.5, 2., 1.};
  auto outData = transposePad<double>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}, 3);
  const auto expected = transposePad<double>({1, 2, 8.5, 4, 26, 6, 44, 8, 62, 10, 65, 12}, 3);
  DataSamples<const int> in(inData.data(), inData.size() / 2, {1, Limits<Index>::inf(), 3}, 2);
  Kernel1d<float> kernel(kernelData, 3);
  DataSamples<double> out(outData.data(), outData.size() / 3, {2, Limits<Index>::inf(), 2}, 3);
  kernel.sparseCorrelate1dTo(in, out);
  for (std::size_t i = 0; i < expected.size(); ++i) {
    BOOST_TEST(outData[i] == expected[i]);
  }
}

template <Index I, typename TIn, typename TKernel, typename TOut>
void testConvolution1d(const TIn& in, const TKernel& kernel, const TOut& expected) {
  const auto out = kernel.template correlateAlong<I, typename TOut::Value>(in);
  BOOST_TEST(out.shape() == in.shape());
  for (const auto& p : out.domain()) {
    printf("%li,%li: %i\n", p[0], p[1], out[p]);
    BOOST_TEST(out[p] == expected[p]);
  }
}

BOOST_AUTO_TEST_CASE(convolve1d_along_test) {
  const Position<3> shape {4, 3, 5};
  Raster<int, 3> in(shape);
  Kernel1d<int, OutOfBoundsCrop> kernel({1, 2, 3}, 1);
  const int value = 10;
  in.fill(value);
  const auto sum = std::accumulate(kernel.begin(), kernel.end(), 0);
  auto alongX = in * sum;
  auto alongY = in * sum;
  auto alongZ = in * sum;
  for (const auto& p : in.domain()) {
    const auto x = p[0];
    const auto y = p[1];
    const auto z = p[2];
    printf("%li,%li,%li:%i\n", x, y, z, in[p]);
    const auto k0 = value * kernel[0];
    const auto k2 = value * kernel[2];
    if (x == 0) {
      alongX[p] -= k0;
    }
    if (y == 0) {
      alongY[p] -= k0;
    }
    if (z == 0) {
      alongZ[p] -= k0;
    }
    if (x == shape[0] - 1) {
      alongX[p] -= k2;
    }
    if (y == shape[1] - 1) {
      alongY[p] -= k2;
    }
    if (z == shape[2] - 1) {
      alongZ[p] -= k2;
    }
  }
  testConvolution1d<0>(in, kernel, alongX);
  testConvolution1d<1>(in, kernel, alongY);
  testConvolution1d<2>(in, kernel, alongZ);
}

BOOST_AUTO_TEST_CASE(standard_separable_convolve2d_test) {
  Raster<int, 2> in({4, 3});
  in.fill(2);
  std::vector<int> kernelData {1, 1, 1};
  Kernel1d<int> kernel(kernelData, 1);
  Raster<int, 2> out(in.shape());
  kernel.correlate2dTo(in, out);
  const std::vector<int> expected {8, 12, 12, 8, 12, 18, 18, 12, 8, 12, 12, 8};
  BOOST_TEST(out.size() == expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i) {
    BOOST_TEST(out.data()[i] == expected[i]);
  }
}

BOOST_AUTO_TEST_CASE(manual_separable_convolve2d_test) {
  Raster<int, 2> in({4, 3});
  in.fill(2);
  std::vector<int> kernelData {1, 1, 1};
  Kernel1d<int> kernel(kernelData, 1);
  auto out = kernel.correlateAlong<0>(in);
  out = kernel.correlateAlong<1>(out);
  const std::vector<int> expected {8, 12, 12, 8, 12, 18, 18, 12, 8, 12, 12, 8};
  BOOST_TEST(out.size() == expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i) {
    BOOST_TEST(out.data()[i] == expected[i]);
  }
}

BOOST_AUTO_TEST_CASE(steped_no_edge_convolve2d_test) {
  printf("\nSTEPPED NO EDGE\n\n");
  Raster<int, 2> in({4 * 3 + 2, 3 * 2 + 2});
  in.fill(2);
  std::vector<int> kernelData {1, 1, 1};
  Kernel1d<int> kernel(kernelData, 1);
  Raster<int, 2> out({4, 3});
  kernel.sparseCorrelate2dTo(in, {{{1, 1}, {4 * 3, 3 * 2}}, {3, 2}}, out);
  const std::vector<int> expected(12, 18);
  BOOST_TEST(out.size() == expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i) {
    BOOST_TEST(out.data()[i] == expected[i]);
  }
}

BOOST_AUTO_TEST_CASE(stepped_front_edge_convolve2d_test) {
  printf("\nSTEPPED FRONT EDGE\n\n");
  Raster<int, 2> in({4 * 3 + 2, 3 * 2 + 2});
  in.fill(2);
  std::vector<int> kernelData {1, 1, 1};
  Kernel1d<int> kernel(kernelData, 1);
  Raster<int, 2> out({4, 3});
  kernel.sparseCorrelate2dTo(in, {{{0, 0}, {3 * 3, 2 * 2}}, {3, 2}}, out);
  const std::vector<int> expected {8, 12, 12, 12, 12, 18, 18, 18, 12, 18, 18, 18};
  BOOST_TEST(out.size() == expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i) {
    BOOST_TEST(out.data()[i] == expected[i]);
  }
}

BOOST_AUTO_TEST_CASE(steped_back_edge_convolve2d_test) {
  printf("\nSTEPPED BACK EDGE\n\n");
  Raster<int, 2> in({4 * 3 + 2, 3 * 2 + 2});
  in.fill(2);
  std::vector<int> kernelData {1, 1, 1};
  Kernel1d<int> kernel(kernelData, 1);
  Raster<int, 2> out({4, 3});
  kernel.sparseCorrelate2dTo(in, {{{4, 3}, {4 * 3 + 1, 3 * 2 + 1}}, {3, 2}}, out);
  const std::vector<int> expected {18, 18, 18, 12, 18, 18, 18, 12, 12, 12, 12, 8};
  BOOST_TEST(out.size() == expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i) {
    BOOST_TEST(out.data()[i] == expected[i]);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
