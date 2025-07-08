// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

namespace Linx {

/**
 * @brief Perform a shallow copy of an image, as a readonly image.
 * 
 * If the input image is aleady readonly, then this is a no-op.
 */
template <typename T, int N, typename TContainer>
KOKKOS_INLINE_FUNCTION decltype(auto) as_readonly(const Image<T, N, TContainer>& in)
{
  if constexpr (std::is_const_v<T>) {
    return in;
  } else {
    using Out = Image<const T, N, typename Rebind<TContainer>::AsReadonly>;
    return Out(Forward {}, in.container());
  }
}

/**
 * @brief Perform a shallow copy of an image, as an atomic image.
 */
template <typename T, int N, typename TContainer>
KOKKOS_INLINE_FUNCTION decltype(auto) as_atomic(const Image<T, N, TContainer>& in)
{
  using Out = Image<T, N, typename Rebind<TContainer>::AsAtomic>;
  return Out(Forward {}, in.container());
}

/**
 * @brief Copy the data to host if on device.
 */
template <typename T, int N, typename TContainer>
decltype(auto) on_host(const Image<T, N, TContainer>& in)
{
  return on_device<Kokkos::HostSpace>(in);
}

/**
 * @brief Copy the data to a given memory space if not already accessible from it.
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace::memory_space, typename T, int N, typename TContainer>
decltype(auto) on_device(const Image<T, N, TContainer>& in)
{
  if constexpr (Kokkos::SpaceAccessibility<TSpace, typename TContainer::memory_space>::accessible) {
    return in;
  } else {
    auto container = Kokkos::create_mirror_view_and_copy(TSpace(), in.container());
    return Image<T, N, decltype(container)>(Forward {}, LINX_MOVE(container));
  }
}

/**
 * @brief Iterator to the beginning of a contiguous image.
 */
template <typename T, int N, typename TContainer>
  requires(is_contiguous<TContainer>())
auto begin(const Image<T, N, TContainer>& image)
{
  return image.data();
}

/**
 * @brief Iterator to the end of a contiguous image.
 */
template <typename T, int N, typename TContainer>
  requires(is_contiguous<TContainer>())
auto end(const Image<T, N, TContainer>& image)
{
  return begin(image) + image.size();
}

} // namespace Linx
