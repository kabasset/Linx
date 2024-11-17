# Project overview

<br/>![Linx logo](doc/diagrams/logo_square.svg)

## License

The Linx library is licensed under [Apache-2.0](LICENSE.txt).

## Build

Build Kokkos:

```sh
cd <kokkos_clone_dir>
git clone https://github.com/kokkos/kokkos.git

mkdir <kokkos_build_dir>
cd <kokkos_build_dir>
cmake <kokkos_clone_dir>/kokkos -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_COMPILER=<kokkos_clone_dir>/bin/nvcc_wrapper -DCMAKE_INSTALL_PREFIX=<kokkos_install_dir> [-DKokkos_ENABLE_SERIAL=ON] [-DKokkos_ENABLE_OPENMP=ON] [-DKokkos_ENABLE_CUDA=ON -DKokkos_ENABLE_CUDA_CONSTEXPR=ON]
make install
```

Build the Linx library and tests:

```sh
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=<kokkos_install_dir>
make
make test
```

## Design concepts

**Data containers**

There are two main data containers: `Sequence` for 1D data, and `Image` for ND data.
Underlying storage is handled by Kokkos by default, and adapts to the target infrastructure.
There is no ordering or contiguity guaratee.
In return, execution is automatically parallelized by Kokkos, including on GPU.

In addition, for interfacing with libraries which require contiguity,
`Raster` is a row-major ordered alternative to `Image` allocated on the host.
It is a standard range (providing `begin()` and `end()`) which eases interfacing with the standard library.
`Image` and `Raster` are also compatible with `std::mdspan`.

Data containers have shared pointer semantics, so that copy is shallow by default.
Deep copy has to be explicit:

```cpp
auto a = Linx::Image(...);
auto b = a;
b *= 2; // Modifies a and b
auto c = +a; // Copies
c *= 2; // Modifies c only
``` 

**Pointwise transforms**

Data containers offer a variety of pointwise transforms which can either modify the data in-place or return new instances or values.
In-place transforms are methods, such as `Image::exp()`, while new-instance transforms are free functions, such as `exp(const Image&)`:

```cpp
auto a = Linx::Image(...);
a.pow(2); // Modifies a in-place
auto a2 = Linx::pow(a, 2); // Creates new instance
auto norm2 = Linx::norm<2>(a); // Returns a value
```

Arbitrarily complex functions can also be applied pointwise with `apply()` or `generate()`:

```cpp
auto a = Linx::Image(...);
auto b = Linx::Image(...);
a.generate(
    "random noise",
    Linx::GaussianRng());
a.apply(
    "logistic function",
    KOKKOS_LAMBDA(auto a_i) { return 1. / (1. + std::exp(-a_i)); });
```

Both methods accept auxiliary data containers as function parameters:

```cpp
auto a = Linx::Image(...);
auto b = Linx::Image(...);
auto c = Linx::Image(...);
a.generate(
    "geometric mean",
    KOKKOS_LAMBDA(auto b_i, auto c_i) { return std::sqrt(b_i * c_i); },
    b, c);
```

**Global transforms**

Global transforms such as Fourier transforms and convolutions are also supported.

```cpp
auto input = Linx::Image(...);
auto kernel = Linx::Image(...);
auto filtered = Linx::Correlation(kernel)(input); // Creates a new instance
auto output = Linx::Image(...);
Linx::Correlation(kernel).transform(input, output); // Fills output
```

**Regional transforms**

There are two ways to work on subsets of elements:
* by slicing some data classes with `slice()`, which return a view of type `Sequence` or `Image` depending on the input type;
* by associating a `Region` to a data class with `patch()`, which results in an object of type `Patch`.

Slices are created from regions of type either `Slice` or `Box`.

Patches accept any type of region, are extremely lightweight and can be moved around when the region is a `Window`, i.e. has shifting capabilities.
Typical windows are `Box`, `Mask` or `Path` and can be used to apply filters.
As opposed to slicing, patching results in an object of type `Patch` instead of simply `Sequence` or `Image`.
Nevertheless, patches are themselves data containers and can be transformed pointwise:

```cpp
auto image = Linx::Image(...):
auto region = Linx::Box(...);
auto patch = Linx::Patch(image, region);
patch.exp(); // Modifies image elements inside region
```

**Pipeline**

Transforms can be combined through a so-called pipeline, using the pipe operator `|` à-la Unix.
Logging and timing tools can be plugged into the pipeline.

```cpp
namespace P = Linx::Pipeline;

auto timer = Linx::TimerLogger();

auto [out] = P::Run("Calibration", timer) // Start a pipeline with embedded timer
    | P::InputFile(darks_path, flats_path) // Read two images
    | P::Batch(Linx::Along<-1>(Linx::Mean())) // Average along the last axis
    | P::OutputFile(mdark_path, mflat_path) // Save intermediate images
    | P::InputFile(light_path) // Read another image
    | P::Apply([](auto l, auto d, auto f) { return (l - d) / f; }) // Apply some pixelwise function
    | P::OutputFile(calibrated_path) // Save calibrated image
    | Linx::Deconvolve(psf); // Filter
```

While running this pipeline, logs are produced, which include the elapsed time of each step.
A typical output could be:

```
Pipeline | Task | Split (ms) | Total (ms)
--- | --- | --- | ---
Calibration | InputFile | 1 | 1
Calibration | Mean | 3 | 4
Calibration | OutputFile | 1 | 5
...
```

**Labels**

Most data classes and services are labeled for logging or debugging purposes, thanks to some `std::string` parameter.
As demonstrated in the snippets above, this also helps documenting the code,
which is why the parameter is purposedly mandatory most of the time.

When possible, labelling is automated, typically when calling simple functions:

```cpp
auto a = Linx::Image("a", ...);
auto b = Linx::sin(a);
assert(b.label() == "sin(a)");

auto k = Linx::Image("kernel", ...);
assert(Linx::Convolution(k).label() == "Convolution(kernel)");
```
