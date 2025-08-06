// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_IMAGE_FUNCS_H
#define LINX_DATA_IMAGE_FUNCS_H

namespace Linx {

/**
 * @brief Perform a shallow copy of an image, as a readonly image.
 * 
 * If the input image is aleady readonly, then this is a no-op.
 */
template <typename T, typename TDomain, typename TContainer>
decltype(auto) as_readonly(const Image<T, TDomain, TContainer>& in)
{
  if constexpr (std::is_const_v<T>) {
    return in;
  } else {
    using Out = Image<const T, TDomain, typename Rebind<TContainer>::AsReadonly>;
    return Out(in.domain(), Forward {}, in.container());
  }
  // FIXME handle shifted in
}

/**
 * @brief Perform a shallow copy of an image, as an atomic image.
 */
template <typename T, typename TDomain, typename TContainer>
decltype(auto) as_atomic(const Image<T, TDomain, TContainer>& in)
{
  using Out = Image<T, TDomain, typename Rebind<TContainer>::AsAtomic>;
  return Out(in.domain(), Forward {}, in.container());
}

/**
 * @brief Copy the data to host if on device.
 */
template <typename T, typename TDomain, typename TContainer>
decltype(auto) on_host(const Image<T, TDomain, TContainer>& in)
{
  return on_device<Kokkos::HostSpace>(in);
}

/**
 * @brief Copy the data to a given memory space if not already accessible from it.
 */
template <
    typename TSpace = Kokkos::DefaultExecutionSpace::memory_space,
    typename T,
    typename TDomain,
    typename TContainer>
decltype(auto) on_device(const Image<T, TDomain, TContainer>& in)
{
  if constexpr (Kokkos::SpaceAccessibility<TSpace, typename TContainer::memory_space>::accessible) {
    return in;
  } else {
    auto container = Kokkos::create_mirror_view_and_copy(TSpace(), in.container());
    return Image<T, TDomain, decltype(container)>(in.domain(), Forward {}, LINX_MOVE(container));
  }
  // FIXME handle shifted in
}

/**
 * @brief Iterator to the beginning of a contiguous image.
 */
template <typename T, typename TDomain, typename TContainer>
  requires(is_contiguous<TContainer>())
auto begin(const Image<T, TDomain, TContainer>& image)
{
  return image.data();
}

/**
 * @brief Iterator to the end of a contiguous image.
 */
template <typename T, typename TDomain, typename TContainer>
  requires(is_contiguous<TContainer>())
auto end(const Image<T, TDomain, TContainer>& image)
{
  return begin(image) + image.size();
}

std::ostream& operator<<(std::ostream& os, const Specialization<Image> auto& image)
{
  os << image.label() << ": " << image.domain() << "\n  ";
  if (image.size() == 0) {
    return os << "[]";
  }

  // FIXME on_host()

  auto stream_row = [&](int tab, auto... is) {
    os << std::string(tab * 2, ' ') << "[ " << image(image.start(0), is...);
    for (int i = image.start(0) + 1; i < image.stop(0); ++i) {
      os << " " << image(i, is...);
    }
    os << " ]";
  };

  auto stream_section = [&](int tab, auto... is) {
    os << "[ ";
    stream_row(0, image.start(1), is...);
    for (int i = image.start(1) + 1; i < image.stop(1); ++i) {
      os << "\n";
      stream_row(tab + 2, i, is...);
    }
    os << " ]";
  };

  auto n = image.rank();
  if (n > 3) {
    for (int i = 0; i < n; ++i) {
      os << "[ ";
    }
    os << image.front() << " ... " << image.back();
    for (int i = 0; i < n; ++i) {
      os << " ]";
    }
  } else if (n == 3) {
    os << "[ ";
    stream_section(1, image.start(2));
    for (int i = image.start(2) + 1; i < image.stop(2); ++i) {
      os << "\n    ";
      stream_section(1, i);
    }
    os << " ]";
  } else if (n == 2) {
    stream_section(0);
  } else if (n == 1) {
    stream_row(0);
  } else {
    os << image.front();
  }
  return os;
}

} // namespace Linx

#endif
