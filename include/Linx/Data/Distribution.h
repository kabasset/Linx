// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_DATA_DISTRIBUTION_H
#define LINX_DATA_DISTRIBUTION_H

#include "Linx/Data/Image.h"

namespace Linx {

namespace Impl {

template <typename TIn, typename TBins, typename TOut>
struct IncrementHistogramBin {
  TIn m_in;
  TBins m_bins;
  TOut m_out;
  int m_bin_count;

  IncrementHistogramBin(TIn in, TBins bins, TOut out) :
      m_in(LINX_MOVE(in)),
      m_bins(LINX_MOVE(bins)),
      m_out(LINX_MOVE(out)),
      m_bin_count(m_out.size())
  {}

  KOKKOS_INLINE_FUNCTION void operator()(auto... is) const
  {
    auto value = m_in(is...);
    if (value >= m_bins(0) && value < m_bins(m_bin_count)) {
      int index = 0;
      while (index < m_bin_count && value >= m_bins(index + 1)) {
        ++index;
      };
      ++m_out(index);
    }
  }
};

} // namespace Impl

template <typename TIn, typename TBins, typename TOut>
void histogram_to(const TIn& in, const TBins& bins, TOut& out)
{
  const auto& atomic_out = as_atomic(out.base());
  const auto& readonly_in = try_as_readonly(in);

  for_each<typename TOut::execution_space>(
      "histogram_to()",
      in.domain(),
      Impl::IncrementHistogramBin(readonly_in, bins, atomic_out));
}

/**
 * @brief Compute the histogram of a view.
 * 
 * @param in The view
 * @param bins The bin bounds
 * 
 * The bins are half-open intervals, such that `histogram[i]` is the number of values
 * both greater or equal to `bins[i]` and strictly less than `bin[i+1]`.
 * Values outside the bins are not counted.
 */
template <typename TOut = int, typename TBins>
auto histogram(const auto& in, const TBins& bins)
{
  auto out = default_init<TOut>(compose_label("histogram", in), bins.size() - 1);
  histogram_to(in, bins, out);
  return out;
}

} // namespace Linx

#endif