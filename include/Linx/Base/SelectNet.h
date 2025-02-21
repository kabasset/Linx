// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_SELECTNET_H
#define LINX_BASE_SELECTNET_H

#include <Kokkos_Swap.hpp>
#include <numeric> // midpoint

namespace Linx {

namespace Impl {

/**
 * @brief Swap two numbers if they are in descending order.
 */
KOKKOS_INLINE_FUNCTION void sort_swap(auto& a, auto& b)
{
  if (a > b) {
    Kokkos::kokkos_swap(a, b);
  }
}

} // namespace Impl

/**
 * @brief Selection network for fixed size arrays.
 * @tparam N The array size if known, or -2 for any even number, or -1 for any odd number, or 0 for any number
 * 
 * If a static implementation is not available for a given size, this falls back to instertion-sort.
 * 
 * @see https://bertdobbelaere.github.io/median_networks.html
 */
template <int N>
struct SelectNet;

template <>
struct SelectNet<1> {
  KOKKOS_INLINE_FUNCTION static decltype(auto) median(auto& in_out)
  {
    return in_out[0];
  }
};

template <>
struct SelectNet<2> {
  KOKKOS_INLINE_FUNCTION static decltype(auto) median(auto& in_out)
  {
    return std::midpoint(in_out[0], in_out[1]);
  }
};

template <>
struct SelectNet<3> {
  KOKKOS_INLINE_FUNCTION static decltype(auto) median(auto& in_out)
  {
    Impl::sort_swap(in_out[0], in_out[1]);
    Impl::sort_swap(in_out[1], in_out[2]);
    Impl::sort_swap(in_out[0], in_out[1]);
    return in_out[1];
  }
};

template <>
struct SelectNet<4> {
  KOKKOS_INLINE_FUNCTION static decltype(auto) median(auto& in_out)
  {
    Impl::sort_swap(in_out[0], in_out[1]);
    Impl::sort_swap(in_out[2], in_out[3]);
    Impl::sort_swap(in_out[0], in_out[2]);
    Impl::sort_swap(in_out[1], in_out[3]);
    return std::midpoint(in_out[1], in_out[2]);
  }
};

template <>
struct SelectNet<5> {
  KOKKOS_INLINE_FUNCTION static decltype(auto) median(auto& in_out)
  {
    Impl::sort_swap(in_out[0], in_out[1]);
    Impl::sort_swap(in_out[3], in_out[4]);
    Impl::sort_swap(in_out[0], in_out[3]);
    Impl::sort_swap(in_out[1], in_out[4]);
    Impl::sort_swap(in_out[1], in_out[2]);
    Impl::sort_swap(in_out[2], in_out[3]);
    Impl::sort_swap(in_out[1], in_out[2]);
    return in_out[2];
  }
};

template <>
struct SelectNet<6> {
  KOKKOS_INLINE_FUNCTION static decltype(auto) median(auto& in_out)
  {
    Impl::sort_swap(in_out[1], in_out[2]);
    Impl::sort_swap(in_out[3], in_out[4]);
    Impl::sort_swap(in_out[0], in_out[1]);
    Impl::sort_swap(in_out[2], in_out[3]);
    Impl::sort_swap(in_out[4], in_out[5]);
    Impl::sort_swap(in_out[1], in_out[2]);
    Impl::sort_swap(in_out[3], in_out[4]);
    Impl::sort_swap(in_out[0], in_out[1]);
    Impl::sort_swap(in_out[2], in_out[3]);
    Impl::sort_swap(in_out[4], in_out[5]);
    Impl::sort_swap(in_out[1], in_out[2]);
    Impl::sort_swap(in_out[3], in_out[4]);
    return std::midpoint(in_out[2], in_out[3]);
  }
};

template <>
struct SelectNet<7> {
  KOKKOS_INLINE_FUNCTION static decltype(auto) median(auto& in_out)
  {
    Impl::sort_swap(in_out[0], in_out[5]);
    Impl::sort_swap(in_out[0], in_out[3]);
    Impl::sort_swap(in_out[1], in_out[6]);
    Impl::sort_swap(in_out[2], in_out[4]);
    Impl::sort_swap(in_out[0], in_out[1]);
    Impl::sort_swap(in_out[3], in_out[5]);
    Impl::sort_swap(in_out[2], in_out[6]);
    Impl::sort_swap(in_out[2], in_out[3]);
    Impl::sort_swap(in_out[3], in_out[6]);
    Impl::sort_swap(in_out[4], in_out[5]);
    Impl::sort_swap(in_out[1], in_out[4]);
    Impl::sort_swap(in_out[1], in_out[3]);
    Impl::sort_swap(in_out[3], in_out[4]);
    return in_out[3];
  }
};

template <>
struct SelectNet<8> {
  KOKKOS_INLINE_FUNCTION static decltype(auto) median(auto& in_out)
  {
    Impl::sort_swap(in_out[0], in_out[2]);
    Impl::sort_swap(in_out[1], in_out[3]);
    Impl::sort_swap(in_out[4], in_out[6]);
    Impl::sort_swap(in_out[5], in_out[7]);
    Impl::sort_swap(in_out[0], in_out[4]);
    Impl::sort_swap(in_out[1], in_out[5]);
    Impl::sort_swap(in_out[2], in_out[6]);
    Impl::sort_swap(in_out[3], in_out[7]);
    Impl::sort_swap(in_out[0], in_out[1]);
    Impl::sort_swap(in_out[2], in_out[4]);
    Impl::sort_swap(in_out[3], in_out[5]);
    Impl::sort_swap(in_out[6], in_out[7]);
    Impl::sort_swap(in_out[2], in_out[3]);
    Impl::sort_swap(in_out[4], in_out[5]);
    Impl::sort_swap(in_out[1], in_out[4]);
    Impl::sort_swap(in_out[3], in_out[6]);
    return std::midpoint(in_out[3], in_out[4]);
  }
};

template <>
struct SelectNet<9> {
  KOKKOS_INLINE_FUNCTION static decltype(auto) median(auto& in_out)
  {
    Impl::sort_swap(in_out[1], in_out[2]);
    Impl::sort_swap(in_out[4], in_out[5]);
    Impl::sort_swap(in_out[7], in_out[8]);
    Impl::sort_swap(in_out[0], in_out[1]);
    Impl::sort_swap(in_out[3], in_out[4]);
    Impl::sort_swap(in_out[6], in_out[7]);
    Impl::sort_swap(in_out[1], in_out[2]);
    Impl::sort_swap(in_out[4], in_out[5]);
    Impl::sort_swap(in_out[7], in_out[8]);
    Impl::sort_swap(in_out[0], in_out[3]);
    Impl::sort_swap(in_out[5], in_out[8]);
    Impl::sort_swap(in_out[4], in_out[7]);
    Impl::sort_swap(in_out[3], in_out[6]);
    Impl::sort_swap(in_out[1], in_out[4]);
    Impl::sort_swap(in_out[2], in_out[5]);
    Impl::sort_swap(in_out[4], in_out[7]);
    Impl::sort_swap(in_out[4], in_out[2]);
    Impl::sort_swap(in_out[6], in_out[4]);
    Impl::sort_swap(in_out[4], in_out[2]);
    return in_out[4];
  }
};

template <>
struct SelectNet<25> {
  KOKKOS_INLINE_FUNCTION static decltype(auto) median(auto& in_out)
  {
    Impl::sort_swap(in_out[0], in_out[1]);
    Impl::sort_swap(in_out[3], in_out[4]);
    Impl::sort_swap(in_out[2], in_out[4]);
    Impl::sort_swap(in_out[2], in_out[3]);
    Impl::sort_swap(in_out[6], in_out[7]);
    Impl::sort_swap(in_out[5], in_out[7]);
    Impl::sort_swap(in_out[5], in_out[6]);
    Impl::sort_swap(in_out[9], in_out[10]);
    Impl::sort_swap(in_out[8], in_out[10]);
    Impl::sort_swap(in_out[8], in_out[9]);
    Impl::sort_swap(in_out[12], in_out[13]);
    Impl::sort_swap(in_out[11], in_out[13]);
    Impl::sort_swap(in_out[11], in_out[12]);
    Impl::sort_swap(in_out[15], in_out[16]);
    Impl::sort_swap(in_out[14], in_out[16]);
    Impl::sort_swap(in_out[14], in_out[15]);
    Impl::sort_swap(in_out[18], in_out[19]);
    Impl::sort_swap(in_out[17], in_out[19]);
    Impl::sort_swap(in_out[17], in_out[18]);
    Impl::sort_swap(in_out[21], in_out[22]);
    Impl::sort_swap(in_out[20], in_out[22]);
    Impl::sort_swap(in_out[20], in_out[21]);
    Impl::sort_swap(in_out[23], in_out[24]);
    Impl::sort_swap(in_out[2], in_out[5]);
    Impl::sort_swap(in_out[3], in_out[6]);
    Impl::sort_swap(in_out[0], in_out[6]);
    Impl::sort_swap(in_out[0], in_out[3]);
    Impl::sort_swap(in_out[4], in_out[7]);
    Impl::sort_swap(in_out[1], in_out[7]);
    Impl::sort_swap(in_out[1], in_out[4]);
    Impl::sort_swap(in_out[11], in_out[14]);
    Impl::sort_swap(in_out[8], in_out[14]);
    Impl::sort_swap(in_out[8], in_out[11]);
    Impl::sort_swap(in_out[12], in_out[15]);
    Impl::sort_swap(in_out[9], in_out[15]);
    Impl::sort_swap(in_out[9], in_out[12]);
    Impl::sort_swap(in_out[13], in_out[16]);
    Impl::sort_swap(in_out[10], in_out[16]);
    Impl::sort_swap(in_out[10], in_out[13]);
    Impl::sort_swap(in_out[20], in_out[23]);
    Impl::sort_swap(in_out[17], in_out[23]);
    Impl::sort_swap(in_out[17], in_out[20]);
    Impl::sort_swap(in_out[21], in_out[24]);
    Impl::sort_swap(in_out[18], in_out[24]);
    Impl::sort_swap(in_out[18], in_out[21]);
    Impl::sort_swap(in_out[19], in_out[22]);
    Impl::sort_swap(in_out[8], in_out[17]);
    Impl::sort_swap(in_out[9], in_out[18]);
    Impl::sort_swap(in_out[0], in_out[18]);
    Impl::sort_swap(in_out[0], in_out[9]);
    Impl::sort_swap(in_out[10], in_out[19]);
    Impl::sort_swap(in_out[1], in_out[19]);
    Impl::sort_swap(in_out[1], in_out[10]);
    Impl::sort_swap(in_out[11], in_out[20]);
    Impl::sort_swap(in_out[2], in_out[20]);
    Impl::sort_swap(in_out[2], in_out[11]);
    Impl::sort_swap(in_out[12], in_out[21]);
    Impl::sort_swap(in_out[3], in_out[21]);
    Impl::sort_swap(in_out[3], in_out[12]);
    Impl::sort_swap(in_out[13], in_out[22]);
    Impl::sort_swap(in_out[4], in_out[22]);
    Impl::sort_swap(in_out[4], in_out[13]);
    Impl::sort_swap(in_out[14], in_out[23]);
    Impl::sort_swap(in_out[5], in_out[23]);
    Impl::sort_swap(in_out[5], in_out[14]);
    Impl::sort_swap(in_out[15], in_out[24]);
    Impl::sort_swap(in_out[6], in_out[24]);
    Impl::sort_swap(in_out[6], in_out[15]);
    Impl::sort_swap(in_out[7], in_out[16]);
    Impl::sort_swap(in_out[7], in_out[19]);
    Impl::sort_swap(in_out[13], in_out[21]);
    Impl::sort_swap(in_out[15], in_out[23]);
    Impl::sort_swap(in_out[7], in_out[13]);
    Impl::sort_swap(in_out[7], in_out[15]);
    Impl::sort_swap(in_out[1], in_out[9]);
    Impl::sort_swap(in_out[3], in_out[11]);
    Impl::sort_swap(in_out[5], in_out[17]);
    Impl::sort_swap(in_out[11], in_out[17]);
    Impl::sort_swap(in_out[9], in_out[17]);
    Impl::sort_swap(in_out[4], in_out[10]);
    Impl::sort_swap(in_out[6], in_out[12]);
    Impl::sort_swap(in_out[7], in_out[14]);
    Impl::sort_swap(in_out[4], in_out[6]);
    Impl::sort_swap(in_out[4], in_out[7]);
    Impl::sort_swap(in_out[12], in_out[14]);
    Impl::sort_swap(in_out[10], in_out[14]);
    Impl::sort_swap(in_out[6], in_out[7]);
    Impl::sort_swap(in_out[10], in_out[12]);
    Impl::sort_swap(in_out[6], in_out[10]);
    Impl::sort_swap(in_out[6], in_out[17]);
    Impl::sort_swap(in_out[12], in_out[17]);
    Impl::sort_swap(in_out[7], in_out[17]);
    Impl::sort_swap(in_out[7], in_out[10]);
    Impl::sort_swap(in_out[12], in_out[18]);
    Impl::sort_swap(in_out[7], in_out[12]);
    Impl::sort_swap(in_out[10], in_out[18]);
    Impl::sort_swap(in_out[12], in_out[20]);
    Impl::sort_swap(in_out[10], in_out[20]);
    Impl::sort_swap(in_out[10], in_out[12]);

    return in_out[12];
  }
};

} // namespace Linx

#endif
