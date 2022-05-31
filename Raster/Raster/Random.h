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

template <typename T, typename TDistribution>
struct ComplexDistribution {

  TDistribution m_distribution;

  template <typename... TArgs>
  ComplexDistribution(TArgs&&... args) : m_distribution(std::forward<TArgs>(args)...) {}

  template <typename TEngine>
  T operator()(TEngine& engine) {
    return m_distribution(engine);
  }
};

template <typename T, typename TDistribution>
struct ComplexDistribution<std::complex<T>, TDistribution> {

  TDistribution m_real, m_imag;

  template <typename... TArgs>
  ComplexDistribution(TArgs&&... args) :
      m_real(std::forward<TArgs>(args).real()...), m_imag(std::forward<TArgs>(args).imag()...) {}

  template <typename TEngine>
  std::complex<T> operator()(TEngine& engine) {
    return {m_real(engine), m_imag(engine)};
  }
};

class RandomGenerator {
public:
  explicit RandomGenerator(Index seed = -1) :
      m_engine(seed == -1 ? std::chrono::system_clock::now().time_since_epoch().count() : seed) {}

protected:
  template <typename T, typename TDistribution>
  T generate(TDistribution& distribution) {
    return distribution(m_engine);
  }

  template <typename T, typename TDistribution>
  T add(T in, TDistribution& distribution) {
    return in + generate<T>(distribution);
  }

  std::mt19937 m_engine;
};

/**
 * @brief Uniform distribution.
 */
template <typename T>
class UniformDistribution : RandomGenerator {

public:
  /**
   * @brief Constructor.
   */
  explicit UniformDistribution(T min = Limits<T>::halfMin(), T max = Limits<T>::halfMax(), Index seed = -1) :
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
 * @brief Gaussian distribution.
 */
template <typename T>
class GaussianDistribution : RandomGenerator {

public:
  /**
   * @brief Constructor.
   */
  explicit GaussianDistribution(T mean = Limits<T>::zero(), T stdev = Limits<T>::one(), Index seed = -1) :
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
 * @brief Poisson distribution.
 */
template <typename T>
class PoissonDistribution : RandomGenerator {

public:
  /**
   * @brief Constructor.
   */
  explicit PoissonDistribution(T mean = Limits<T>::zero(), Index seed = -1) :
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

} // namespace Cnes

#endif
