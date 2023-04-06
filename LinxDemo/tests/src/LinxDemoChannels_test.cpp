// @copyright 2022, Antoine Basset (CNES)
// This file is part of Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LinxCore/Raster.h"

#include <boost/test/unit_test.hpp>

//! [Rgb struct]
// An RGB (red, green, blue) color where each channel takes values between 0 and 255.
struct Rgb {

  // The associated image type
  using Image = Linx::Raster<Rgb, 2>;

  // The components
  unsigned char r, g, b;

  // operator==() is mandatory to work with rasters
  bool operator==(const Rgb& other) const {
    return r == other.r && g == other.g && b == other.b;
  }
};
//! [Rgb struct]

//! [Hsv struct]
// A HSV (hue, saturation, value) color with H in [0, 360), and S and V in [0, 1].
struct Hsv {

  using Image = Linx::Raster<Hsv, 2>;

  double h, s, v;

  bool operator==(const Hsv& other) const {
    return h == other.h && s == other.s && v == other.v;
  }
};
//! [Hsv struct]

/**
 * @brief Convert an RGB pixel into a HSV pixel.
 */
Hsv rgbToHsv(const Rgb& rgb) {

  const double normalization = 1. / 255.;
  const double r = normalization * rgb.r;
  const double g = normalization * rgb.g;
  const double b = normalization * rgb.b;

  Hsv hsv;

  const auto min = std::min({r, g, b});
  const auto max = std::max({r, g, b});
  const auto delta = max - min;

  if (max == 0.) {
    return hsv;
  }

  hsv.v = max;

  if (delta == 0.) {
    return hsv;
  }

  hsv.s = delta / max;

  const auto dr = (60. * (max - r) + 180. * delta) / delta;
  const auto dg = (60. * (max - g) + 180. * delta) / delta;
  const auto db = (60. * (max - b) + 180. * delta) / delta;

  if (r == max) {
    hsv.h = db - dg;
  } else if (g == max) {
    hsv.h = 120. + dr - db;
  } else {
    hsv.h = 240. + dg - dr;
  }

  if (hsv.h < 0.) {
    hsv.h += 360.;
  } else if (hsv.h >= 360.) {
    hsv.h -= 360.;
  }

  return hsv;
}

void rgbDataToHsvData(unsigned char* rgb, double* hsv) {
  const auto out = rgbToHsv(Rgb {rgb[0], rgb[1], rgb[2]});
  hsv[0] = out.h;
  hsv[1] = out.s;
  hsv[2] = out.v;
}

std::string toString(const Rgb& rgb) {
  return std::to_string(rgb.r) + "R " + std::to_string(rgb.g) + "G " + std::to_string(rgb.b) + "B";
}

std::string toString(const Hsv& hsv) {
  return std::to_string(hsv.h) + "H " + std::to_string(hsv.s) + "S " + std::to_string(hsv.v) + "V";
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(LinxDemoChannels_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(color_struct_test) {

  //! [Input Rgb]
  static constexpr Rgb turquoise {64, 224, 208};
  Rgb::Image rgb({640, 480});
  rgb.fill(turquoise);
  //! [Input Rgb]

  //! [Output Hsv]
  Hsv::Image hsv(rgb.shape());
  hsv.generate(rgbToHsv, rgb);
  //! [Output Hsv]

  std::cout << toString(rgb[{0, 0}]) << " = " << toString(hsv[{0, 0}]) << std::endl;
}

BOOST_AUTO_TEST_CASE(color_along_0_test) {

  //! [CxyImage]
  using RgbXyImage = Linx::Raster<unsigned char, 3>;
  using HsvXyImage = Linx::Raster<double, 3>;
  //! [CxyImage]

  //! [Input Cxy]
  RgbXyImage rgb({3, 640, 480});
  for (auto it = rgb.begin(); it != rgb.end();) {
    *it++ = 64; // R
    *it++ = 224; // G
    *it++ = 208; // B
  }
  //! [Input Cxy]

  //! [Output Cxy]
  HsvXyImage hsv(rgb.shape());
  auto hsvIt = hsv.begin();
  // Loop by steps of 3 pixels
  for (auto rgbIt = rgb.begin(); rgbIt != rgb.end(); rgbIt += 3, hsvIt += 3) {
    rgbDataToHsvData(rgbIt, hsvIt);
  }
  //! [Output Cxy]

  const Rgb rgb0 {rgb[0], rgb[1], rgb[2]};
  const Hsv hsv0 {hsv[0], hsv[1], hsv[2]};
  std::cout << toString(rgb0) << " = " << toString(hsv0) << std::endl;
}

BOOST_AUTO_TEST_CASE(color_along_2_test) {

  //! [XycImage]
  using XyRgbImage = Linx::Raster<unsigned char, 3>;
  //! [XycImage]

  //! [Input Xyc]
  XyRgbImage rgb({640, 480, 3});
  auto r = rgb.section(0);
  auto g = rgb.section(1);
  auto b = rgb.section(2);
  r.fill(64);
  g.fill(224);
  b.fill(208);
  //! [Input Xyc]
  const Rgb in0 {rgb[{0, 0, 0}], rgb[{0, 0, 1}], rgb[{0, 0, 2}]};

  //! [Output Xyc]
  r *= 0.2125;
  g *= 0.7154;
  b *= 0.0721;
  //! [Output Xyc]
  const Rgb out0 {rgb[{0, 0, 0}], rgb[{0, 0, 1}], rgb[{0, 0, 2}]};

  std::cout << toString(in0) << " -> " << toString(out0) << std::endl;
}

BOOST_AUTO_TEST_CASE(vector_along_2_test) {

  //! [Vector image]
  using HyperspectralCube = Linx::Raster<double, 3>;
  using IntegratedImage = Linx::Raster<double, 2>;
  //! [Vector image]

  Linx::Index n = 100; // Or read from command line
  //! [Input vector]
  HyperspectralCube cube({n, 640, 480});
  cube.generate(Linx::GaussianNoise<double>());
  std::vector<double> weights(n, 1.41);
  //! [Input vector]

  //! [Output scalar]
  IntegratedImage image({cube.length(1), cube.length(2)});
  auto cubeIt = cube.begin();
  for (auto imageIt = image.begin(); imageIt != image.end(); ++imageIt, cubeIt += n) {
    *imageIt = std::inner_product(weights.begin(), weights.end(), cubeIt, 0.);
  }
  //! [Output scalar]

  std::cout << "Integrated flux = " << image[0] << std::endl;
}

BOOST_AUTO_TEST_CASE(raster_of_raster_test) {

  //! [Raster of raster]
  using NestedRaster = Linx::Raster<Linx::Raster<int>>;
  NestedRaster nested({4, 3});

  for (auto& p : nested.domain()) {
    const auto shape = p + 1;
    auto& raster = nested[p];
    raster = Linx::Raster<int>(shape).range();
    std::cout << "Raster at " << p << " = " << raster << std::endl;
    BOOST_TEST(raster.at(-1) == shapeSize(shape) - 1);
  }
  //! [Raster of raster]
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
