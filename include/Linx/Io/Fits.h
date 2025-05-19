// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_IO_FITS_H
#define LINX_IO_FITS_H

#include "Linx/Base/Exceptions.h"
#include "Linx/Data/Image.h"
#include "Linx/Io/Exceptions.h"

#include <filesystem>
#include <fitsio.h>
#include <stdexcept>
#include <string>

namespace Linx {

/**
 * @brief CFITSIO-thrown error.
 */
class CfitsioError : public Exception {
public:

  /**
     * @brief Constructor.
     */
  CfitsioError(const std::string& context, fitsfile* fptr, int status) : Exception(context)
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

  /**
   * @brief Throw if `status` is not nul.
   */
  static void may_throw(const std::string& context, fitsfile* fptr, int status)
  {
    if (status != 0) {
      throw CfitsioError(context, fptr, status);
    }
  }
};

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
   * @brief Constructor.
   */
  Fits(const std::filesystem::path& path, FileMode mode = FileMode::Create) : m_path(path), m_mode(mode), m_fptr()
  {
    FileNotFound::may_throw(m_path, m_mode);
    PathAlreadyExists::may_throw(m_path, m_mode);

    int status = 0;
    std::string overwrited_path = "!";
    switch (mode) {
      case FileMode::Read:
        fits_open_file(&m_fptr, m_path.c_str(), READONLY, &status);
        break;
      case FileMode::Edit:
        fits_open_file(&m_fptr, m_path.c_str(), READWRITE, &status);
        break;
      case FileMode::Create:
      case FileMode::Temporary:
        fits_create_file(&m_fptr, m_path.c_str(), &status);
        break;
      case FileMode::Overwrite:
        overwrited_path += m_path;
        fits_create_file(&m_fptr, overwrited_path.c_str(), &status);
        break;
      case FileMode::Write:
        if (std::filesystem::is_regular_file(m_path)) {
          fits_open_file(&m_fptr, m_path.c_str(), READWRITE, &status);
        } else {
          fits_create_file(&m_fptr, m_path.c_str(), &status);
        }
        break;
    }
    CfitsioError::may_throw("Cannot open file", m_fptr, status);
  }

  /**
   * @brief Constructor.
   */
  Fits(const std::filesystem::path& path, char mode) : Fits(path, FileMode(mode)) {}

  /**
   * @brief Destructor.
   * 
   * This simply calls `close()` while discarding potential exceptions.
   */
  ~Fits()
  {
    try {
      close();
    } catch (...) {
    }
  }

  /**
   * @brief Close the file.
   * 
   * Destroy the file if the mode is `Temporary`.
   */
  void close()
  {
    int status = 0;
    if (m_mode == FileMode::Temporary) {
      fits_delete_file(m_fptr, &status);
    } else {
      fits_close_file(m_fptr, &status);
    }
    m_fptr = nullptr;
    CfitsioError::may_throw("Cannot close file", m_fptr, status);
  }

  /**
   * @brief Get the file path.
   */
  const std::filesystem::path& path() const
  {
    return m_path; // FIXME from m_fptr
  }

  /**
   * @brief Read an image at given (0-based) HDU index.
   */
  template <typename T, int N>
  Image<T, N> read(Index hdu = 0)
  {
    std::stringstream label;
    label << m_path.stem() << '[' << hdu << ']';
    int status = 0;
    int naxis = 0;
    fits_movabs_hdu(m_fptr, hdu + 1, nullptr, &status);
    fits_get_img_dim(m_fptr, &naxis, &status);
    Position<N> shape("shape", naxis);
    fits_get_img_size(m_fptr, naxis, shape.data(), &status);
    Raster<T, N> out("raster", shape);
    fits_read_img(m_fptr, typecode<T>(), 1, out.size(), nullptr, out.data(), nullptr, &status);
    CfitsioError::may_throw("Cannot read HDU", m_fptr, status);
    return Image<T, N>(label.str(), shape).copy(out); // FIXME optimize
  }

  /**
   * @brief Write an image as a new FITS file.
   * @param raster The raster to be written
   * @param mode `x` to create a new file, `w` to create or overwrite, `a` to append an HDU
   */
  template <typename TImage>
  void write(TImage in)
  {
    int status = 0;
    std::vector<long> shape(in.rank());
    for (int i = 0; i < in.rank(); ++i) {
      shape[i] = in.extent(i);
    }
    fits_create_img(m_fptr, image_typecode<typename TImage::element_type>(), in.rank(), shape.data(), &status);
    if (in.size() > 0) {
      write_pixels(m_fptr, in, Position<-1>("first", in.rank()));
    }
    CfitsioError::may_throw("Cannot write HDU", m_fptr, status);
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

  /**
   * @brief Write pixels in an existing image HDU.
   * @param in The image to be written
   * @param start The position of the first pixel in the file
   */
  template <typename TIn, typename TStart>
  void write_pixels(fitsfile* m_fptr, const TIn& in, const TStart& start)
  {
    using T = typename TIn::element_type;
    static constexpr auto n = TIn::n;
    const auto& h = on_host(in);
    const auto raster = Raster<T, n>(compose_label("raster", in.label()), in.shape()).copy_from(h);
    write_pixels(m_fptr, raster, start);
  }

  template <typename T, int N, typename TStart>
  void write_pixels(fitsfile* m_fptr, const Raster<T, N>& in, const TStart& start)
  {
    int status = 0;
    fits_write_img(m_fptr, typecode<T>(), 1, in.size(), in.data(), &status);
    CfitsioError::may_throw("Cannot write pixels", m_fptr, status);
  }

private:

  /**
   * @brief Get CFITSIO's typecode.
   */
  template <typename T>
  static constexpr int typecode()
  {
    if constexpr (std::is_same_v<T, bool>) {
      return TBYTE;
    } else if constexpr (std::is_integral_v<T>) {
      constexpr bool sign = std::is_signed_v<T>;
      if constexpr (sizeof(T) == 1) {
        return sign ? TSBYTE : TBYTE;
      }
      using Signed = std::make_signed_t<T>;
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
  FileMode m_mode;
  fitsfile* m_fptr;
};

} // namespace Linx

#endif
