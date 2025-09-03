// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_IMAGE_FUNCS_H
#define LINX_DATA_IMAGE_FUNCS_H

#include "Linx/Base/OutputStream.h"

namespace Linx {

/**
 * @brief Perform a shallow copy of an image, as a readonly image.
 * 
 * If the input image is aleady readonly, then this is a no-op.
 */
template <typename T, typename TDomain, typename TView>
decltype(auto) as_readonly(const Image<T, TDomain, TView>& in)
{
  if constexpr (std::is_const_v<T>) {
    return in;
  } else {
    using Out = Image<const T, TDomain, typename Rebind<TView>::add_const>;
    return Out(in.domain(), Forward {}, in.base());
  }
  // FIXME handle shifted in
}

/**
 * @brief Perform a shallow copy of an image, as an atomic image.
 */
template <typename T, typename TDomain, typename TView>
decltype(auto) as_atomic(const Image<T, TDomain, TView>& in)
{
  using Out = Image<T, TDomain, typename Rebind<TView>::add_atomic>;
  return Out(in.domain(), Forward {}, in.base());
}

/**
 * @brief Copy the data to host if on device.
 */
template <typename T, typename TDomain, typename TView>
decltype(auto) on_host(const Image<T, TDomain, TView>& in)
{
  return on_device<Kokkos::HostSpace>(in);
}

/**
 * @brief Copy the data to a given memory space if not already accessible from it.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace::memory_space, typename T, typename TDomain, typename TView>
decltype(auto) on_device(const Image<T, TDomain, TView>& in)
{
  if constexpr (Kokkos::SpaceAccessibility<TSpace, typename TView::memory_space>::accessible) {
    return in;
  } else {
    auto view = Kokkos::create_mirror_view_and_copy(TSpace(), in.base());
    return Image<T, TDomain, decltype(view)>(in.domain(), Forward {}, LINX_MOVE(view));
  }
  // FIXME handle shifted in
}

/**
 * @brief Iterator to the beginning of a contiguous image.
 */
template <typename T, typename TDomain, typename TView>
  requires(is_contiguous<TView>())
auto begin(const Image<T, TDomain, TView>& image)
{
  return image.data();
}

/**
 * @brief Iterator to the end of a contiguous image.
 */
template <typename T, typename TDomain, typename TView>
  requires(is_contiguous<TView>())
auto end(const Image<T, TDomain, TView>& image)
{
  return begin(image) + image.size();
}

/**
 * @brief Align a contiguous-domain 1D data view along an axis, reshaping it into an ND image.
 * @tparam I The axis to align the array along
 * @tparam N The rank of the output image (-1 is not supported)
 * 
 * The input can be a sequence or a 1D image, possibly offset.
 * The output is an image or an offset image.
 */
template <Index I, Index N = I + 1, typename TIn>
auto along(const TIn& in)
{
  const auto& r = root(in);
  auto start = vec<Dimension(N)>(0);
  auto stop = vec<Dimension(N)>(1);
  start[I] = r.start(0);
  stop[I] = r.stop(0);
  auto out = no_init<typename TIn::value_type, typename TIn::execution_space>(r.label(), Box(start, stop));
  const auto& out_on_host = on_host(out);
  for (auto i : get<0>(in.domain())) {
    auto p = vec<Dimension(N)>(0);
    p[I] = i;
    out_on_host.at(p) = r(i);
  }
  Kokkos::deep_copy(out.base(), out_on_host.base());
  return out;
}

std::ostream& operator<<(std::ostream& os, const Specialization<Image> auto& in)
{
  auto label = in.label().empty() ? "<Image>" : in.label();

  os << label << ": " << in.domain() << "\n  ";
  if (in.size() == 0) {
    return os << "[]";
  }

  const int radius = PrintLimit::edge(os);
  const int diameter = radius == 0 ? std::numeric_limits<int>::max() : 2 * radius + 1;

  const auto& in_on_host = on_host(in);

  auto stream_row = [&](int tab, auto... is) {
    auto start = in_on_host.start(0);
    auto stop = in_on_host.stop(0);
    // First value
    os << std::string(tab * 2, ' ') << "[ " << in_on_host(start, is...);
    if (in_on_host.extent(0) <= diameter) {
      // Remaining values
      for (auto i = start + 1; i < stop; ++i) {
        os << " " << in_on_host(i, is...);
      }
    } else {
      // Remaining values in front edge
      for (auto i = start + 1; i < start + radius; ++i) {
        os << " " << in_on_host(i, is...);
      }
      // Ellipsis
      os << " ...";
      // Values in back edge
      for (auto i = stop - radius; i < stop; ++i) {
        os << " " << in_on_host(i, is...);
      }
    }
    os << " ]";
  };

  auto stream_section = [&](int tab, auto... is) {
    auto start = in_on_host.start(1);
    auto stop = in_on_host.stop(1);
    // First row
    os << "[ ";
    stream_row(0, start, is...);
    if (in_on_host.extent(1) <= diameter) {
      // Remaining rows
      for (int i = start + 1; i < stop; ++i) {
        os << "\n";
        stream_row(tab + 2, i, is...);
      }
    } else {
      // Remaining rows in front edge
      for (auto i = start + 1; i < start + radius; ++i) {
        os << "\n";
        stream_row(tab + 2, i, is...);
      }
      // Ellipsis
      os << "\n" << std::string(tab * 2 + 4, ' ') << "...";
      // Rows in back edge
      for (auto i = stop - radius; i < stop; ++i) {
        os << "\n";
        stream_row(tab + 2, i, is...);
      }
    }
    os << " ]";
  };

  long n = in_on_host.rank();
  if (n > 3) {
    for (auto i = 0; i < n; ++i) {
      os << "[ ";
    }
    os << in_on_host.front() << " ... " << in_on_host.back();
    for (auto i = 0; i < n; ++i) {
      os << " ]";
    }
  } else if (n == 3) {
    auto start = in_on_host.start(2);
    auto stop = in_on_host.stop(2);
    // First section
    os << "[ ";
    stream_section(1, start);
    if (in_on_host.extent(2) <= diameter) {
      // Remaining sections
      for (auto i = start + 1; i < stop; ++i) {
        os << "\n    ";
        stream_section(1, i);
      }
    } else {
      // Remaining sections in front edge
      for (auto i = start + 1; i < start + radius; ++i) {
        os << "\n    ";
        stream_section(1, i);
      }
      // Ellipsis
      os << "\n" << std::string(4, ' ') << "...";
      for (auto i = stop - radius; i < stop; ++i) {
        os << "\n    ";
        stream_section(1, i);
      }
    }
    os << " ]";
  } else if (n == 2) {
    stream_section(0);
  } else if (n == 1) {
    stream_row(0);
  } else {
    os << in_on_host.front();
  }
  return os;
}

} // namespace Linx

#endif
