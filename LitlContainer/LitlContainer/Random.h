// @copyright 2022, Antoine Basset (CNES)
// This file is part of Litl <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LITLCONTAINER_RANDOM_H
#define _LITLCONTAINER_RANDOM_H

#include "LitlTypes/TypeUtils.h"

#include <algorithm>
#include <chrono>
#include <map>
#include <random>

namespace Litl {

/**
 * @ingroup concepts
 * @requirements{RandomNoise}
 * @brief Random noise generator requirements for use in containers.
 * 
 * A random noise generator of type `T` is a class which provides two methods:
 * - `T operator()()` generates a random value;
 * - `T operator()(T in)` applies some (additive or not) random noise to an input value `in`.
 * 
 * These methods are used by `DataContainer::generate()` and `DataContainer::apply()`, respectively,
 * when they are called with a random noise generator as parameter.
 */

/**
 * @ingroup random
 * @brief Helper class to create a complex distribution from two real distributions.
 */
template <typename T, typename TDistribution>
class ComplexDistribution {

public:
  /**
   * @brief Forwarding constructor.
   * 
   * If `T` is complex, `args.real()` and `args.imag()` are forwarded
   * to the constructors of some real and imaginary distributions, respectively.
   * For example, a complex Gaussian distribution can be initialized
   * with mean `{0, 100}` and standard deviation `{1, 15}`,
   * which will produce two real Gaussian distributions under the hood:
   * one real distribution of mean 0 and standard deviation 1,
   * and one imaginary distribution of mean 100 and standard deviation 15.
   */
  template <typename... TArgs>
  ComplexDistribution(TArgs&&... args) : m_distribution(std::forward<TArgs>(args)...) {}

  /**
   * @brief Generate some value.
   * 
   * If `T` is complex, two values are generated,
   * one from the real distribution and the other from the imaginary distribution.
   */
  template <typename TEngine>
  T operator()(TEngine& engine) {
    return m_distribution(engine);
  }

  /**
   * @brief Reset the distribution state.
   */
  void reset() {
    m_distribution.reset();
  }

private:
  /**
   * @brief The distribution.
   */
  TDistribution m_distribution;
};

/// @cond

template <typename T, typename TDistribution>
class ComplexDistribution<std::complex<T>, TDistribution> {

public:
  template <typename... TArgs>
  ComplexDistribution(TArgs&&... args) :
      m_real(std::forward<TArgs>(args).real()...), m_imag(std::forward<TArgs>(args).imag()...) {}

  template <typename TEngine>
  std::complex<T> operator()(TEngine& engine) {
    return {m_real(engine), m_imag(engine)};
  }

  void reset() {
    m_real.reset();
    m_imag.reset();
  }

private:
  TDistribution m_real, m_imag;
};

/// @endcond

/**
 * @ingroup random
 * @brief Helper class to simplify implementation of random noise generators.
 * 
 * Random noise generators can extend this class and provide `operator()()`s
 * relying on `generate()` and `add()` for random value generation and additive noise generation, respectively.
 * Member `m_engine` is available for more complex uses.
 */
class RandomGenerator {
public:
  /**
   * @brief Constructor.
   * @param seed The random engine seed or -1 for using current time.
   */
  explicit RandomGenerator(std::size_t seed = -1) :
      m_engine(seed != std::size_t(-1) ? seed : std::chrono::system_clock::now().time_since_epoch().count()) {}

protected:
  /**
   * @brief Generate some random value.
   */
  template <typename T, typename TDistribution>
  T generate(TDistribution& distribution) {
    return distribution(m_engine);
  }

  /**
   * @brief Add some random value to a given input.
   */
  template <typename T, typename TDistribution>
  T add(T in, TDistribution& distribution) {
    return in + generate<T>(distribution);
  }

  /**
   * @brief The random engine.
   */
  std::mt19937 m_engine;
};

/**
 * @ingroup random
 * @brief Uniform noise generator.
 * @satisfies{RandomNoise}
 */
template <typename T>
class UniformNoise : RandomGenerator {

public:
  /**
   * @brief Constructor.
   */
  explicit UniformNoise(T min = Limits<T>::halfMin(), T max = Limits<T>::halfMax(), std::size_t seed = -1) :
      RandomGenerator(seed), m_distribution(min, max) {}

  /**
   * @brief Generate value.
   */
  T operator()() {
    return generate<T>(m_distribution);
  }

  /**
   * @brief Apply additive noise.
   */
  T operator()(T in) {
    return add<T>(in, m_distribution);
  }

private:
  ComplexDistribution<
      T,
      std::conditional_t<
          std::is_integral<T>::value,
          std::uniform_int_distribution<T>,
          std::uniform_real_distribution<typename TypeTraits<T>::Scalar>>>
      m_distribution;
};

/**
 * @ingroup random
 * @brief Gaussian noise generator.
 * @satisfies{RandomNoise}
 */
template <typename T>
class GaussianNoise : RandomGenerator {

public:
  /**
   * @brief Constructor.
   */
  explicit GaussianNoise(T mean = Limits<T>::zero(), T stdev = Limits<T>::one(), std::size_t seed = -1) :
      RandomGenerator(seed), m_distribution(mean, stdev) {}

  /**
   * @brief Generate value.
   */
  T operator()() {
    return generate<T>(m_distribution);
  }

  /**
   * @brief Apply additive noise.
   */
  T operator()(T in) {
    return add<T>(in, m_distribution);
  }

private:
  using Scalar = std::conditional_t<std::is_integral<T>::value, double, typename TypeTraits<T>::Scalar>;
  ComplexDistribution<T, std::normal_distribution<Scalar>> m_distribution;
};

/**
 * @ingroup random
 * @brief Poisson noise generator.
 * 
 * 
 * @satisfies{RandomNoise}
 * 
 * @warning
 * The generation of one value might require a number of drawings by the random engine
 * which depends on the Poisson distribution mean.
 * This has no consequences for random value generation, where the mean is constant,
 * but impacts shot noise drawing reproducibility.
 * See `StablePoissonNoise` as a solution to this potential issue.
 */
template <typename T>
class PoissonNoise : RandomGenerator {

public:
  /**
   * @brief Constructor.
   */
  explicit PoissonNoise(T mean = Limits<T>::zero(), std::size_t seed = -1) :
      RandomGenerator(seed), m_distribution(mean) {}

  /**
   * @brief Generate value.
   */
  T operator()() {
    return generate<T>(m_distribution);
  }

  /**
   * @brief Apply shot noise.
   */
  T operator()(T in) {
    decltype(m_distribution) distribution(in);
    return generate<T>(distribution);
  }

private:
  using Scalar = std::conditional_t<std::is_integral<T>::value, T, long>;
  ComplexDistribution<T, std::poisson_distribution<Scalar>> m_distribution;
};

/**
 * @ingroup random
 * @brief Poisson noise generator which is robust to local changes.
 * 
 * If, at a given set of indices, two containers have the same values,
 * then the noises applied at these indices are identical.
 * For example, applying stable Poisson noise to containers `{0, 1, 2, 3}` and `{10, 100, 2, 30}`
 * yiels the exact same value at index 2.
 * 
 * This is in contrast to `PoissonNoise`, which is faster but produces noise
 * which is statistically-independent but process-dependent on the previous realizations,
 * because sampling one value generally requires several draws from the random engine.
 * Here, the random engine is reseeded before each noise drawing.
 * 
 * As opposed to random _noise_ generation,
 * `PoissonNoise` and `StablePoissonNoise` have the same implementation for random _value_ generation.
 * 
 * Because it is meant for reproducibility,
 * `StablePoissonNoise` cannot be instantiated with a random seed.
 * 
 * @satisfies{RandomNoise}
 */
template <typename T>
class StablePoissonNoise : RandomGenerator {

public:
  /**
   * @brief Fixed-seed constructor.
   * 
   * @warning
   * By default, the seed is 0 and not -1,
   * because this noise generator is intended for reproducible results.
   */
  explicit StablePoissonNoise(T mean = Limits<T>::zero(), std::size_t seed = 0) :
      RandomGenerator(seed), m_distribution(mean), m_seeder(seed) {}

  /**
   * @brief Generate value.
   */
  T operator()() {
    return generate<T>(m_distribution);
  }

  /**
   * @brief Apply shot noise.
   */
  T operator()(T in) {
    auto seed = m_seeder();
    PoissonNoise<T> noise(in, seed); // TODO can we just reseed m_engine?
    return noise();
  }

private:
  using Scalar = std::conditional_t<std::is_integral<T>::value, T, long>;
  ComplexDistribution<T, std::poisson_distribution<Scalar>> m_distribution;
  std::minstd_rand m_seeder;
};

/**
 * @ingroup random
 * @brief Impulse noise generator (encompasses salt-and-pepper noise).
 * 
 * The call operator randomly generates discrete values with given probabilities.
 * If the sum of probabilities _s_ is less than 1,
 * then the input value is untouched with probability 1 - _s_.
 * If the sum of probabilities _s_ is greater than 1,
 * then the input probabilities are normalized by _s_.
 * 
 * For salt-and-pepper noise, see maker `saltAndPepper()`.
 * 
 * @satisfies{RandomNoise}
 */
template <typename T>
class ImpulseNoise : RandomGenerator {

public:
  /**
   * @brief Single value constructor.
   */
  explicit ImpulseNoise(T value, double probability, std::size_t seed = -1) :
      ImpulseNoise({{value, probability}}, seed) {}

  /**
   * @brief Multiple values constructor.
   */
  explicit ImpulseNoise(const std::map<T, double>& valueProbabilities, std::size_t seed = -1) :
      RandomGenerator(seed), m_values(values(valueProbabilities)), m_distribution(distribution(valueProbabilities)) {}

  /**
   * @brief Make a salt and pepper noise generator.
   */
  static ImpulseNoise saltAndPepper(
      double pSalt,
      double pPepper,
      T salt = Limits<T>::max(),
      T pepper = Limits<T>::min(),
      std::size_t seed = -1) {
    return ImpulseNoise({{salt, pSalt}, {pepper, pPepper}}, seed);
  }

  /**
   * @brief Apply impulse noise.
   * 
   * Generate discete values with constructor-provided probabilities,
   * and return the input value the rest of the time (if _s_ < 1).
   */
  T operator()(T in = Limits<T>::zero()) {
    auto index = generate<std::size_t>(m_distribution);
    return index < m_values.size() ? m_values[index] : in;
  }

private:
  /**
   * @brief Extract the values from the value-probability map.
   */
  static std::vector<T> values(const std::map<T, double>& valueProbabilities) {
    std::vector<T> out(valueProbabilities.size());
    std::transform(valueProbabilities.begin(), valueProbabilities.end(), out.begin(), [](auto vp) {
      return vp.first;
    });
    return out;
  }

  /**
   * @brief Extract the weights from the value-probability map.
   */
  static std::vector<double> weights(const std::map<T, double>& valueProbabilities) {
    const auto count = valueProbabilities.size();
    std::vector<double> out(count + 1);
    auto& nullProbability = out[count];
    nullProbability = 1;
    std::transform(valueProbabilities.begin(), valueProbabilities.end(), out.begin(), [&](auto vp) {
      nullProbability -= vp.second;
      return vp.second;
    });
    if (nullProbability <= std::numeric_limits<double>::epsilon() * count) {
      out.resize(count); // Discard null hypothesis
    }
    return out;
  }

  /**
   * @brief Construct the distribution from the value-probability map.
   */
  static std::discrete_distribution<std::size_t> distribution(const std::map<T, double>& valueProbabilities) {
    const auto w = weights(valueProbabilities);
    return std::discrete_distribution<std::size_t>(w.begin(), w.end());
  }

  /**
   * @brief The impulse values.
   */
  std::vector<T> m_values;

  /**
   * @brief The impulse value index distribution.
   */
  std::discrete_distribution<std::size_t> m_distribution;
};

} // namespace Litl

#endif
