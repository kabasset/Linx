/// @copyright 2022, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXIO_FITS_H
#define _LINXIO_FITS_H

#include "Linx/Data/Raster.h"

#include <filesystem>
#include <fitsio.h>
#include <stdexcept>
#include <string>

namespace Linx {

/**
 * @brief FITS file reader/writer.
 */
class Fits {

public:
  class Error : public std::exception {
  public:
    Error(const std::string& context, int status) : m_message(context) {

      m_message += '\n';

      float version = 0;
      fits_get_version(&version);
      m_message += "CFITSIO v" + std::to_string(version) + ' ';

      char text[FLEN_ERRMSG];
      text[0] = '\0';
      fits_get_errstatus(status, text);
      m_message += "Error " + std::to_string(status) + ": " + text + '\n';

      char message[80];
      while (fits_read_errmsg(message) != 0) {
        m_message += message;
      };
    }

    const char* what() const noexcept override {
      return m_message.c_str();
    }

  private:
    std::string m_message;
  };

  /**
   * @brief Constructor.
   */
  Fits(const std::filesystem::path& path) : m_path(path), m_fptr(nullptr), m_status(0) {}

  /**
   * @brief Get the file path.
   */
  const std::filesystem::path& path() const {
    return m_path;
  }

  /**
   * @brief Read an image at given (0-based) HDU index.
   */
  template <typename TRaster>
  TRaster read(Index hdu = 0) {
    int naxis = 0;
    fits_open_file(&m_fptr, m_path.c_str(), READONLY, &m_status);
    fits_movabs_hdu(m_fptr, hdu + 1, nullptr, &m_status);
    fits_get_img_dim(m_fptr, &naxis, &m_status);
    Position<TRaster::Dimension> shape(naxis);
    fits_get_img_size(m_fptr, naxis, shape.data(), &m_status);
    TRaster out(shape);
    fits_read_img(m_fptr, typecode<typename TRaster::Value>(), 1, out.size(), nullptr, out.data(), nullptr, &m_status);
    fits_close_file(m_fptr, &m_status);
    if (m_status != 0) {
      throw Error("Cannot read file " + m_path.string(), m_status);
    }
    m_fptr = nullptr;
    return out;
  }

  /**
   * @brief Write an image as a new FITS file.
   */
  template <typename TRaster>
  void write(TRaster raster) { // FIXME const
    fits_create_file(&m_fptr, m_path.c_str(), &m_status);
    auto shape = raster.shape();
    fits_create_img(m_fptr, imageTypecode<typename TRaster::Value>(), raster.dimension(), shape.data(), &m_status);
    fits_write_img(m_fptr, typecode<typename TRaster::Value>(), 1, raster.size(), raster.data(), &m_status);
    fits_close_file(m_fptr, &m_status);
    if (m_status != 0) {
      throw Error("Cannot write file " + m_path.string(), m_status);
    }
    m_fptr = nullptr;
  }

  /**
   * @brief Get the BITPIX of a given type.
   */
  template <typename T>
  static constexpr int bitpix() {
    if constexpr (std::is_integral_v<T>) {
      return 8 * static_cast<int>(sizeof(T));
    }
    if constexpr (std::is_floating_point_v<T>) {
      return -8 * static_cast<int>(sizeof(T));
    }
    return 0;
  }

private:
  /**
   * @brief Get CFITSIO's typecode.
   */
  template <typename T>
  static constexpr int typecode() {
    if constexpr (std::is_integral_v<T>) {
      constexpr bool sign = std::is_signed_v<T>;
      using Signed = std::make_signed_t<T>;
      if constexpr (sizeof(T) == 1) {
        return sign ? TSBYTE : TBYTE;
      }
      if constexpr (std::is_same_v<Signed, short>) {
        return sign ? TSHORT : TUSHORT;
      }
      if constexpr (std::is_same_v<Signed, int>) {
        return sign ? TINT : TUINT;
      }
      if constexpr (std::is_same_v<Signed, long>) {
        return sign ? TLONG : TULONG;
      }
      if constexpr (std::is_same_v<Signed, long long>) {
        return sign ? TLONGLONG : TULONGLONG;
      }
    }
    if constexpr (std::is_floating_point_v<T>) {
      if constexpr (sizeof(T) == 4) {
        return TFLOAT;
      }
      if constexpr (sizeof(T) == 8) {
        return TDOUBLE;
      }
    }
    return 0;
  }

  /**
   * @brief Get CFITSIO's image typecode.
   */
  template <typename T>
  static constexpr int imageTypecode() {
    if constexpr (std::is_integral_v<T>) {
      constexpr bool sign = std::is_signed_v<T>;
      if constexpr (sizeof(T) == 1) {
        return sign ? SBYTE_IMG : BYTE_IMG;
      }
      if constexpr (sizeof(T) == 2) {
        return sign ? SHORT_IMG : USHORT_IMG;
      }
      if constexpr (sizeof(T) == 4) {
        return sign ? LONG_IMG : ULONG_IMG;
      }
      if constexpr (sizeof(T) == 8) {
        return sign ? LONGLONG_IMG : ULONGLONG_IMG;
      }
    }
    if constexpr (std::is_floating_point_v<T>) {
      if constexpr (sizeof(T) == 4) {
        return FLOAT_IMG;
      }
      if constexpr (sizeof(T) == 8) {
        return DOUBLE_IMG;
      }
    }
    return 0;
  }

private:
  std::filesystem::path m_path;
  fitsfile* m_fptr;
  int m_status;
};

} // namespace Linx

#endif
