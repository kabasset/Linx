// SPDX-FileCopyrightText: Copyright (C) 2024, Antoine Basset
// SPDX-PackageSourceInfo: https://github.com/kabasset/Linx
// SPDX-License-Identifier: Apache-2.0

#ifndef LINX_BASE_RANDOM_H
#define LINX_BASE_RANDOM_H

#include "Linx/Base/Slice.h"
#include "Linx/Base/Types.h"
#include "Linx/Base/mixins/Arithmetic.h"

#include <Kokkos_Core.hpp>
#include <Kokkos_Random.hpp>
#include <boost/math/special_functions.hpp>
#include <chrono> // For random seed

namespace Linx {

/**
 * @brief Strong type for seeding RNGs.
 */
class Seed {
public:

  /**
   * @brief Constructor.
   */
  LINX_UFUNC explicit Seed(Index value = -1) : m_value(value) {}

  /**
   * @brief Value.
   */
  LINX_UFUNC Index operator()() const
  {
    return m_value;
  }

private:

  Index m_value; ///< The value
};

/**
 * @brief Pool of random number generators for parallel usage.
 */
template <typename T, typename TPool = Kokkos::Random_XorShift64_Pool<Kokkos::DefaultExecutionSpace>>
class RngPool {
public:

  /**
   * @brief Generator, to be used with RAII.
   */
  class Generator {
  public:

    /**
     * @brief Constructor (acquire state).
     */
    KOKKOS_INLINE_FUNCTION explicit Generator(const auto& pool) : m_pool(pool), m_gen(m_pool.get_state()) {}

    /**
     * @brief Destructor (free state).
     */
    KOKKOS_INLINE_FUNCTION ~Generator()
    {
      m_pool.free_state(m_gen);
    }

    /**
     * @brief Draw a single, uniformly distributed sample.
     */
    template <typename TOut>
    KOKKOS_INLINE_FUNCTION TOut uniform(TOut infimum, TOut supremum)
    {
      return Kokkos::rand<decltype(m_gen), TOut>::draw(m_gen, infimum, supremum);
    }

  private:

    const TPool& m_pool; ///< The Kokkos pool
    typename TPool::generator_type m_gen; ///< The Kokkos generator
  };

  /**
   * @brief Constructor.
   * @param seed The random seed, or -1 to get a time-dependent seed
   */
  RngPool(Index seed = -1) : m_pool(seed != -1 ? seed : std::chrono::system_clock::now().time_since_epoch().count()) {}

  /**
   * @brief Draw a single, uniformly distributed sample.
   * 
   * For more complex cases, use `generator()`.
   */
  KOKKOS_INLINE_FUNCTION T uniform(T infimum, T supremum) const
  {
    return generator().uniform(infimum, supremum);
  }

  /**
   * @brief Get a generator.
   * 
   * The generator acquires and releases pool a state through RAII, for example:
   * 
   * \code
   * double u;
   * {
   *   auto gen = pool.generator();
   *   u = gen.uniform(0, 1);
   * }
   * \endcode
   */
  KOKKOS_INLINE_FUNCTION auto generator() const
  {
    return Generator(m_pool);
  }

private:

  TPool m_pool;
};

/**
 * @ingroup random
 * @brief Uniform probability distribution.
 * 
 * For integral types, the interval is closed;
 * for other types, the supremum is exclusive.
 */
template <typename T>
class UniformDistribution {
public:

  using Interval = std::conditional_t<std::is_integral_v<T>, Segment<T>, Slice<T>>; ///< The type of interval

  /**
   * @brief Constructor.
   */
  LINX_UFUNC explicit UniformDistribution(T infimum = Limits<T>::min(), T supremum = Limits<T>::max()) :
      m_slice(infimum, supremum)
  {}

  /**
   * @brief Constructor.
   */
  LINX_UFUNC explicit UniformDistribution(Interval slice) : m_slice(LINX_MOVE(slice)) {}

  /**
   * @brief Lower bound (inclusive).
   */
  LINX_UFUNC T infimum() const
  {
    return m_slice.pred().infimum;
  }

  /**
   * @brief Upper bound (exclusive).
   */
  LINX_UFUNC T supremum() const
  {
    return m_slice.pred().supremum;
  }

  /**
   * @brief Probability density or mass function.
   */
  LINX_UFUNC double operator()(auto x) const
  {
    return m_slice.contains(x) ? 1. / m_slice.size() : 0.;
  }

  /**
   * @brief Cumulative density function.
   */
  LINX_UFUNC double cdf(auto x) const
  {
    const auto& a = infimum();
    const auto& b = supremum();

    if (x <= a) {
      return 0.;
    }

    if (x >= b) {
      return 1.;
    }

    return double(T(x) - a + std::is_integral_v<T>) / m_slice.size();
  }

private:

  Slice<T> m_slice; ///< Interval
};

/**
 * @ingroup random
 * @brief Uniform random number generator.
 * 
 * \code
 * auto noise = Linx::generate<100>("noise", Linx::UniformRng(0., 1.));
 * \endcode
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
class UniformRng : public CopyArithmeticMixin<const T, UniformRng<T, TSpace>> {
public:

  using value_type = const T;

  /**
   * @brief Fixed-seed constructor.
   */
  explicit UniformRng(Seed seed, T infimum = Limits<T>::min(), T supremum = Limits<T>::max()) :
      m_distribution(infimum, supremum),
      m_pool(seed())
  {}

  /**
   * @brief Automatic-seed constructor.
   */
  explicit UniformRng(T infimum = Limits<T>::min(), T supremum = Limits<T>::max()) :
      UniformRng(Seed(), infimum, supremum)
  {}

  /**
   * @brief Label.
   */
  constexpr std::string label() const
  {
    return "Uniform"; // TODO parameters
  }

  /**
   * @brief The distribution.
   */
  LINX_UFUNC const auto& distribution() const
  {
    return m_distribution;
  }

  /**
   * @brief Sample. 
   */
  KOKKOS_INLINE_FUNCTION T operator()(auto&&...) const
  {
    return m_pool.uniform(m_distribution.infimum(), m_distribution.supremum() + std::is_integral_v<T>);
  }

private:

  UniformDistribution<T> m_distribution; ///< Distribution parameters
  using Value = std::conditional_t<std::is_same_v<T, bool>, char, T>; ///< Specific handling
  RngPool<Value, Kokkos::Random_XorShift64_Pool<TSpace>> m_pool; ///< RNG pool
};

/**
 * @ingroup random
 * @brief Gaussian probability distribution.
 * 
 * \code
 * auto noise = Linx::generate<100>("noise", Linx::GaussianRng(100, 15));
 * \endcode
 */
template <typename T>
class GaussianDistribution {
public:

  /**
   * @brief Constructor.
   */
  LINX_UFUNC explicit GaussianDistribution(T mu = 0, T sigma = 1) : m_mu(mu), m_sigma(sigma) {}

  /**
   * @brief Mean.
   */
  LINX_UFUNC T mean() const
  {
    return m_mu;
  }

  /**
   * @brief Standard deviation. 
   */
  LINX_UFUNC T stddev() const
  {
    return m_sigma;
  }

  /**
   * @brief Probability density function.
   */
  LINX_UFUNC double operator()(const auto& x) const
  {
    const auto u = x - m_mu;
    const auto norm = std::numbers::inv_sqrtpi * std::numbers::sqrt2 * 0.5 / m_sigma;
    const auto constant = -0.5 / (m_sigma * m_sigma);
    return norm * std::exp(u * u * constant);
  }

  /**
   * @brief Cumulative density function.
   */
  LINX_UFUNC double cdf(const auto& x) const
  {
    return .5 * (1. + std::erf((x - m_mu) / (std::numbers::sqrt2 * m_sigma)));
  }

private:

  T m_mu; ///< Mean
  T m_sigma; ///< Standard deviation
};

/**
 * @ingroup random
 * @brief Gaussian random number generator.
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
class GaussianRng : public CopyArithmeticMixin<const T, GaussianRng<T, TSpace>> {
public:

  using value_type = const T;

  /**
   * @brief Fixed-seed constructor.
   */
  explicit GaussianRng(Seed seed, T mu = 0, T sigma = 1) : m_distribution(mu, sigma), m_pool(seed()) {}

  /**
   * @brief Automatic-seed constructor.
   */
  explicit GaussianRng(T mu = 0, T sigma = 1) : GaussianRng(Seed(), mu, sigma) {}

  /**
   * @brief Label.
   */
  constexpr std::string label() const
  {
    return "Gaussian"; // TODO parameters
  }

  /**
   * @brief The distribution.
   */
  LINX_UFUNC const auto& distribution() const
  {
    return m_distribution;
  }

  /**
   * @brief Sample using the Box-Muller method.
   */
  KOKKOS_INLINE_FUNCTION T operator()(auto&&...) const
  {
    double u;
    double theta;
    {
      auto gen = m_pool.generator();
      u = gen.uniform(-1., 0.); // for sampling over (0, 1] instead of [0, 1)
      theta = gen.uniform(0., 2 * std::numbers::pi);
    }

    const double r = Kokkos::sqrt(-2 * Kokkos::log(-u)); // -u in (0, 1]
    const double x = r * Kokkos::cos(theta);

    if constexpr (is_complex<T>()) { // Get two variables at once
      const double y = r * Kokkos::sin(theta);
      return T(x, y) * m_distribution.stddev() + m_distribution.mean();
    } else {
      return x * m_distribution.stddev() + m_distribution.mean();
    }
  }

private:

  GaussianDistribution<T> m_distribution; ///< Distribution parameters
  RngPool<T, Kokkos::Random_XorShift64_Pool<TSpace>> m_pool; ///< RNG pool
};

/**
 * @ingroup random
 * @brief Poisson probability distribution.
 */
template <typename T>
class PoissonDistribution {
public:

  /**
   * @brief Constructor.
   */
  LINX_UFUNC explicit PoissonDistribution(T lambda) : m_lambda(lambda) {}

  /**
   * @brief Mean.
   */
  LINX_UFUNC T mean() const
  {
    return m_lambda;
  }

  /**
   * @brief Probability mass function.
   */
  LINX_UFUNC double operator()(const std::integral auto& k) const
  {
    return std::pow(m_lambda, k) * std::exp(-m_lambda) / boost::math::factorial(k);
  }

  /**
   * @brief Cumulative density function.
   */
  LINX_UFUNC double cdf(const auto& x) const
  {
    auto floor = std::floor(x);
    return boost::math::gamma_q(floor + 1, m_lambda);
  }

private:

  T m_lambda; ///< Mean
};

/**
 * @ingroup random
 * @brief Poisson random number generator.
 */
template <typename T, typename TSpace = Kokkos::DefaultExecutionSpace>
class PoissonRng : public CopyArithmeticMixin<const T, PoissonRng<T, TSpace>> {
public:

  using value_type = const T;

  /**
   * @brief Fixed-seed constructor.
   */
  explicit PoissonRng(Seed seed, T lambda) : m_lambda(lambda), m_pool(seed()) {}

  /**
   * @brief Automatic-seed constructor.
   */
  explicit PoissonRng(T lambda) : PoissonRng(Seed(), lambda) {}

  /**
   * @brief Label.
   */
  constexpr std::string label() const
  {
    return "Poisson"; // TODO parameters
  }

  /**
   * @brief Sample.
   */
  KOKKOS_INLINE_FUNCTION T operator()(auto&&...) const
  {
    if (m_lambda <= 0) {
      return 0;
    }

    auto u = m_pool.uniform(0., 1.);

    if (u == 0) {
      return 0;
    }

    // TODO support complex?
    auto p = std::exp(-m_lambda);
    auto cp = 0.0;
    T k {};
    while (cp < u) {
      cp += p;
      ++k;
      p *= m_lambda / k;
    }

    return k - 1;
  }

private:

  double m_lambda;
  RngPool<T, Kokkos::Random_XorShift64_Pool<TSpace>> m_pool; ///< RNG pool
};

/**
 * @ingroup random
 * @brief Poisson noise generator.
 * 
 * As opposed to many implementations of Poisson noise generators,
 * this generator draws only once to generate one value,
 * which means that the noise is effectively iid.:
 * 
 * \code
 * auto a = Linx::Sequence<int>({1, 10, 100, 1000}).transform(Linx::PoissonNoise(42));
 * auto b = Linx::Sequence<int>({1, 10, 0, 1000}).transform(Linx::PoissonNoise(42));
 * assert(a[0] == b[0]);
 * assert(a[1] == b[1]);
 * assert(a[3] == b[3]); // This fails in many implementations
 * \endcode
 */
template <typename TSpace = Kokkos::DefaultExecutionSpace> // FIXME PoissonRng<Forward, TSpace>?
class PoissonNoise {
public:

  /**
   * @brief Constructor.
   */
  explicit PoissonNoise(Seed seed = Seed()) : m_pool(seed()) {}

  /**
   * @brief Sample from a given mean value.
   */
  KOKKOS_INLINE_FUNCTION Index operator()(double lambda) const
  {
    // For stability, generate u even when lambda <= 0
    auto u = m_pool.uniform(0., 1.);

    if (lambda <= 0 || u == 0) {
      return 0;
    }

    // TODO support complex?
    auto p = std::exp(-lambda);
    auto cp = 0.0;
    Index k {};
    while (cp < u) {
      cp += p;
      ++k;
      p *= lambda / k;
    }

    return k - 1;
  }

private:

  RngPool<double, Kokkos::Random_XorShift64_Pool<TSpace>> m_pool; ///< RNG pool
};

} // namespace Linx

#endif
