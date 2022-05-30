# Change log

# 1.0 (wrt. EleFits 5.0.1)

## New features

* FFTW-wrapper as `DftPlan` class
* Linear filtering through `Kernel` class
* `Raster` has Euclidian ring arithmetic (not only vector space arithmetic)
* `Raster` supports mathematical functions (`abs`, `min`, `sin`, `exp`...)
* New `Raster` specialization `AlignedRaster` supports owning and sharing memory-aligned data
* New alias `Index` for `long`, mostly for documentation purpose
* Header-only library

## Cleaning

* Major documentation update, with various demos and deeper explanations
* Example code blocks are extracted from unit tests
