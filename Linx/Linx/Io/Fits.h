/// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: Apache-2.0

#ifndef _LINXIO_FITS_H
#define _LINXIO_FITS_H

#include "Linx/Data/Raster.h"
#include "Linx/Io/Exceptions.h"

#include <filesystem>
#include <fitsio.h>
#include <stdexcept>
#include <string>

namespace Linx {

/**
 * @brief FITS file reader/writer.
 * 
 * This is a simple handler able to read or write an image HDU.
 * For example, streaming and keyword records are not handled.
 * For anything more complex, see EleFits: https://cnes.github.io/EleFits/
 */
class Fits {
public:

  /**
   * @brief FITS-specific `FileFormatError`.
   */
  class Error : public Exception {
  public:

    /**
     * @brief Constructor.
     */
    Error(const std::string& context, const std::filesystem::path& path, int status) : Exception(context, path)
    {
      float version = 0;
      fits_get_version(&version);
      append("CFITSIO v" + std::to_string(version));

      char text[FLEN_ERRMSG];
      text[0] = '\0';
      fits_get_errstatus(status, text);
      append("Error " + std::to_string(status) + ": " + text);

      char message[80];
      while (fits_read_errmsg(message) != 0) {
        append(message);
      };
    }
  };

  /**
   * @brief Constructor.
   */
  Fits(const std::filesystem::path& path) : m_path(path) {}

  /**
   * @brief Get the file path.
   */
  const std::filesystem::path& path() const
  {
    return m_path;
  }

  /**
   * @brief Read an image at given (0-based) HDU index.
   */
  template <typename TRaster>
  TRaster read(Index hdu = 0)
  {
    FileNotFoundError::may_throw(m_path);
    int status = 0;
    fitsfile* fptr;
    int naxis = 0;
    fits_open_file(&fptr, m_path.c_str(), READONLY, &status);
    if (status != 0) {
      throw FileFormatError("Cannot read file", m_path);
    }
    fits_movabs_hdu(fptr, hdu + 1, nullptr, &status);
    fits_get_img_dim(fptr, &naxis, &status);
    Position<TRaster::Dimension> shape(naxis);
    fits_get_img_size(fptr, naxis, shape.data(), &status);
    TRaster out(shape);
    fits_read_img(fptr, typecode<typename TRaster::Value>(), 1, out.size(), nullptr, out.data(), nullptr, &status);
    fits_close_file(fptr, &status);
    if (status != 0) {
      throw Error("Cannot read file", m_path, status);
    }
    fptr = nullptr;
    return out;
  }

  /**
   * @brief Write an image as a new FITS file.
   * @param raster The raster to be written
   * @param mode `x` to create a new file, `w` to create or overwrite, `a` to append an HDU
   */
  template <typename TRaster>
  void write(TRaster raster, char mode = 'x')
  {
    int status = 0;
    fitsfile* fptr;
    std::string path = "!"; // For overwriting
    switch (mode) {
      case 'x':
        PathExistsError::may_throw(m_path);
        fits_create_file(&fptr, m_path.c_str(), &status);
        break;
      case 'w':
        path += m_path;
        fits_create_file(&fptr, path.c_str(), &status);
        break;
      case 'a':
        FileNotFoundError::may_throw(m_path);
        fits_open_file(&fptr, m_path.c_str(), READWRITE, &status);
        break;
      default:
        throw Exception("Unknown write mode", std::string(1, mode));
    }
    if (status != 0) {
      throw FileFormatError("Cannot write file", m_path);
    }
    auto shape = raster.shape();
    fits_create_img(fptr, image_typecode<typename TRaster::Value>(), raster.dimension(), shape.data(), &status);
    if (raster.size() > 0) {
      std::vector<std::decay_t<typename TRaster::Value>> nonconst(raster.begin(), raster.end());
      fits_write_img(fptr, typecode<typename TRaster::Value>(), 1, raster.size(), nonconst.data(), &status);
    }
    fits_close_file(fptr, &status);
    if (status != 0) {
      throw Error("Cannot write file", m_path, status);
    }
    fptr = nullptr;
  }

  /**
   * @brief Get the BITPIX of a given type.
   */
  template <typename T>
  static constexpr int bitpix()
  {
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
  static constexpr int typecode()
  {
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
  static constexpr int image_typecode()
  {
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
};

} // namespace Linx

#endif
