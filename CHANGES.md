# Change Log

# 1.0 (wrt. EleFits 5)

## Breaking changes

* Namespace change (`Linx`)
* `camelCase` replaced with `snake_case`
* `Region` replaced with `Box`
* `Dim` renamed as `Dimension`
* `Position::max()` replaced with `Position::inf()`

## New features

* FFTW-wrapper `DftPlan`
* Linear, rank and morphology filtering through `SimpleFilter` class
* Composite filters with `FilterAgg` and/or `FilterSeq`
* Extrapolation and interpolation with `Extrapolation` and `Interpolation`
* Affine transformations with `Affinity`
* `Raster` has Euclidean ring arithmetic (not only vector space arithmetic)
* Containers support mathematical functions (`abs()`, `min()`, `sin()`, `exp()`...)
* Containers support filling methods (`fill()`, `range()`...),
  random values can be generated with `generate()`,
  and random noise can be added with `apply()`
* New `Raster` specialization `AlignedRaster` supports owning and sharing memory-aligned data
* Containers support `std::valarray` as a data holder
* 1D container `Vector` generalizes `Position` with template value type
* Alias `Index` for `long`, mostly for documentation purpose
* Header-only library

## Cleaning

* Specializable `ArithmeticMixin` simplifies code factorization
* Unit tests are generated for each supported type
* Major documentation update, with various demos and deeper explanations
* Example code blocks are extracted from unit tests
