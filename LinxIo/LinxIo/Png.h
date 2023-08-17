/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXIO_PNG_H
#define _LINXIO_PNG_H

#include <png.h>
#include <string>

namespace Linx {

class Png {
public:

  Png(const std::string& filename);

  bool accept();

  template <typename TRaster>
  TRaster read();

  template <typename TRaster>
  void write(const TRaster& raster);

private:
};

} // namespace Linx

#endif
