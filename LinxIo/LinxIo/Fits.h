/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXIO_FITS_H
#define _LINXIO_FITS_H

#include "LinxCore/Raster.h"

#include <fitsio.h>
#include <string>

namespace Linx {

class Fits {

public:
  Fits(const std::string& filename) : m_filename(filename), m_fptr(nullptr), m_status(0) {}

  template <typename TRaster>
  TRaster read() {
    int naxis = 0;
    fits_open_file(&m_fptr, m_filename.c_str(), READONLY, &m_status);
    fits_get_img_dim(m_fptr, &naxis, &m_status);
    Position<TRaster::Dimension> shape(naxis);
    fits_get_img_size(m_fptr, naxis, shape.data(), &m_status);
    TRaster out(shape);
    fits_read_img(
        m_fptr,
        TBYTE, // FIXME
        1,
        out.size(),
        nullptr,
        out.data(),
        nullptr,
        &m_status);
    fits_close_file(m_fptr, &m_status);
    // FIXME may throw
    m_fptr = nullptr;
    return out;
  }

  template <typename TRaster>
  void write(TRaster raster) { // FIXME const
    fits_create_file(&m_fptr, m_filename.c_str(), &m_status);
    auto shape = raster.shape();
    fits_create_img(
        m_fptr,
        BYTE_IMG, // FIXME
        raster.dimension(),
        shape.data(),
        &m_status);
    fits_write_img(
        m_fptr,
        TBYTE, // FIXME
        1,
        raster.size(),
        raster.data(),
        &m_status);
    fits_close_file(m_fptr, &m_status);
    // FIXME may throw
    m_fptr = nullptr;
  }

private:
  std::string m_filename;
  fitsfile* m_fptr;
  int m_status;
};

} // namespace Linx

#endif
