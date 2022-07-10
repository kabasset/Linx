/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLIO_TIFF_H
#define _LITLIO_TIFF_H

#include <string>
#include <tiff.h>

namespace Litl {

class Tiff {

public:
  Tiff(const std::string& filename) : m_tif(TIFFOpen(filename.c_str(), "r")) { // FIXME mode
  }

  ~Tiff() {
    TIFFClose(m_tif);
  }

  bool accept();

  template <typename TRaster>
  TRaster read() {
    std::uint32_t width, height;
    TIFFGetField(m_tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(m_tif, TIFFTAG_IMAGELENGTH, &height);
    TRaster out({width, height});
    TIFFReadRGBAImage(m_tif, w, h, out.data(), 0);
    return out;
  }

  template <typename TRaster>
  void write(const TRaster& raster);

private:
  TIFF* m_tif;
};

} // namespace Litl

#endif
