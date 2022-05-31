// Copyright (C) 2022, Antoine Basset
// This file is part of Cnes.Raster <github.com/kabasset/Raster>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _RASTER_RANDOM_H
#define _RASTER_RANDOM_H

#include "Raster/Position.h" // Index
#include "RasterTypes/TypeUtils.h" // Complexifier

#include <algorithm>
#include <chrono>
#include <random>

namespace Cnes {

/**
 * @ingroup concepts
 * @requirements{RandomDistribution}
 * @brief Random distribution requirements for use in containers.
 * @details
 * A random distribution of type `T` is a class which provides two methods:
 * - `T operator()()` generates a random value;
 * - `T operator()(T)` applies some (additive or not) random noise to an input value.
 * 
 * These methods are used by `DataContainer::generate()` and `DataContainer::apply()`, respectively,
 * when they are called with a random distribution as parameter.
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
 * @brief Helper class to simplify implementation of random distributions.
 * @details
 * Random distributions can extend this class and provide `operator()()`s
 * relying on `generate()` and `add()` for random value generation and additive noise generation, respectively.
 * Member `m_engine` is available for more complex uses.
 */
class RandomGenerator {
public:
  /**
   * @brief Constructor.
   * @param seed The random engine seed.
   */
  template <typename TSeed>
  explicit RandomGenerator(TSeed&& seed) : m_engine(std::forward<TSeed>(seed)) {}

  /**
   * @brief Constructor.
   */
  explicit RandomGenerator() : RandomGenerator(std::chrono::system_clock::now().time_since_epoch().count()) {}

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
 * @brief Uniform distribution.
 * @satisfies{RandomDistribution}
 */
template <typename T>
class UniformDistribution : RandomGenerator {

public:
  /**
   * @brief Constructor.
   */
  explicit UniformDistribution(T min = Limits<T>::halfMin(), T max = Limits<T>::halfMax()) :
      RandomGenerator(), m_distribution(min, max) {}

  /**
   * @brief Constructor.
   */
  template <typename TSeed>
  explicit UniformDistribution(T min, T max, TSeed&& seed) :
      RandomGenerator(std::forward<TSeed>(seed)), m_distribution(min, max) {}

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
 * @brief Gaussian distribution.
 * @satisfies{RandomDistribution}
 */
template <typename T>
class GaussianDistribution : RandomGenerator {

public:
  /**
   * @brief Constructor.
   */
  explicit GaussianDistribution(T mean = Limits<T>::zero(), T stdev = Limits<T>::one()) :
      RandomGenerator(), m_distribution(mean, stdev) {}

  /**
   * @brief Constructor.
   */
  template <typename TSeed>
  explicit GaussianDistribution(T mean, T stdev, TSeed&& seed) :
      RandomGenerator(std::forward<TSeed>(seed)), m_distribution(mean, stdev) {}

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
 * @brief Poisson distribution.
 * @satisfies{RandomDistribution}
 */
template <typename T>
class PoissonDistribution : RandomGenerator {

public:
  /**
   * @brief Constructor.
   */
  explicit PoissonDistribution(T mean = Limits<T>::zero()) : RandomGenerator(), m_distribution(mean) {}

  /**
   * @brief Constructor.
   */
  template <typename TSeed>
  explicit PoissonDistribution(T mean, TSeed&& seed) :
      RandomGenerator(std::forward<TSeed>(seed)), m_distribution(mean) {}

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

} // namespace Cnes

#endif
