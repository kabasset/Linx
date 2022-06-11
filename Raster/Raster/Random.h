// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_RANDOM_H
#define _RASTER_RANDOM_H

#include "Raster/Position.h" // Index
#include "RasterTypes/TypeUtils.h" // Complexifier

#include <algorithm>
#include <chrono>
#include <map>
#include <random>

namespace Cnes {

/**
 * @ingroup concepts
 * @requirements{RandomNoise}
 * @brief Random noise generator requirements for use in containers.
 * @details
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
   * @details
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
   * @details
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
 * @details
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
          std::uniform_real_distribution<typename Complexifier<T>::Component>>>
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
  using Component = std::conditional_t<std::is_integral<T>::value, double, typename Complexifier<T>::Component>;
  ComplexDistribution<T, std::normal_distribution<Component>> m_distribution;
};

/**
 * @ingroup random
 * @brief Poisson noise generator.
 * @details
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
  using Component = std::conditional_t<std::is_integral<T>::value, T, long>;
  ComplexDistribution<T, std::poisson_distribution<Component>> m_distribution;
};

/**
 * @ingroup random
 * @brief Poisson noise generator which is robust to local changes.
 * @details
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
   * @details
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
  using Component = std::conditional_t<std::is_integral<T>::value, T, long>;
  ComplexDistribution<T, std::poisson_distribution<Component>> m_distribution;
  std::minstd_rand m_seeder;
};

/**
 * @ingroup random
 * @brief Impulse noise generator (encompasses salt-and-pepper noise).
 * @details
 * Randomly generate discrete values with given probabilities.
 * If the sum of probabilities _s_ is less than 1,
 * then the input value is untouched with probability 1 - _s_.
 * If the sum of probabilities _s_ is greater than 1,
 * then the input probabilities are normalized by _s_.
 * 
 * For salt-and-pepper noise, simply use as initialization `{{min, pMin}, {max, pMax}}`,
 * e.g. `ImpulseNoise<T> noise({{Limits<T>::min(), 0.1}, {Limits<T>::max(), 0.2});`.
 * 
 * @satisfies{RandomNoise}
 */
template <typename T>
class ImpulseNoise : RandomGenerator {

public:
  /**
   * @brief Constructor.
   */
  explicit ImpulseNoise(const std::map<T, double>& valueProbabilities, std::size_t seed = -1) :
      RandomGenerator(seed), m_values(values(valueProbabilities)), m_distribution(weights(valueProbabilities)) {}

  /**
   * @brief Apply impulse noise.
   */
  T operator()(T in = Limits<T>::zero()) {
    auto index = generate<std::size_t>(m_distribution);
    return index < m_values.size() ? m_values[index] : in;
  }

private:
  static std::vector<T> values(const std::map<T, double>& valueProbabilities) {
    std::vector<T> out(valueProbabilities.size());
    std::transform(valueProbabilities.begin(), valueProbabilities.end(), out.begin(), [](auto vp) {
      return vp.first;
    });
    return out;
  }

  static std::vector<T> weights(const std::map<T, double>& valueProbabilities) {
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

  std::vector<T> m_values;
  std::discrete_distribution<std::size_t> m_distribution;
};

} // namespace Cnes

#endif
