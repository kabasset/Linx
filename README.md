# Project overview

<br/>![Linx logo](doc/diagrams/logo_square.svg)

## Introduction

Linx stands for cross-platform, extensible ND image laboratory.
This is a template-heavy library made for handling multi-dimensional data and associated signal processing.
By default, memory is allocated on available acceleration devices (typically GPU's)
and transfers between the host and devices should be minimal.
Using templates opens the library for extension, in order to maximize compatibility with established libraries
and enable implementing additional features in user code with zero performance cost.
Internally, we also rely heavily on metaprogramming to enable as many compile-time optimizations as we can.
The library comes with utilities for building processing workflows, and a few demonstration executables.

## License

The Linx library is licensed under [Apache-2.0](LICENSE.txt).

## Build

Linx relies on C++20 features.
It depends on Kokkos and CFITSIO.
While CFITSIO binaries are available, Kokkos has to be built from sources for performance.
Kokkos' serial execution space is mandatory, while OpenMP and Cuda backends are optional.

```sh
export KOKKOS_SOURCE_DIR=<kokkos_source_dir>
export KOKKOS_BUILD_DIR=<kokkos_build_dir>
export KOKKOS_INSTALL_DIR=<kokkos_install_dir>

cd $KOKKOS_SOURCE_DIR
git clone https://github.com/kokkos/kokkos.git

mkdir $KOKKOS_BUILD_DIR
cd $KOKKOS_BUILD_DIR
cmake $KOKKOS_SOURCE_DIR/kokkos -DCMAKE_CXX_STANDARD=20 \
  -DCMAKE_CXX_COMPILER=$KOKKOS_SOURCE_DIR/bin/nvcc_wrapper -DCMAKE_INSTALL_PREFIX=$KOKKOS_INSTALL_DIR \
  -DKokkos_ENABLE_SERIAL=ON \
  [-DKokkos_ENABLE_OPENMP=ON] \
  [-DKokkos_ENABLE_CUDA=ON -DKokkos_ENABLE_CUDA_CONSTEXPR=ON]
make install
```

Assuming the Linx sources have been cloned or downloaded,
the following commands can be run from the source directory for building, testing and installing the library and executables.

```sh
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=$KOKKOS_INSTALL_DIR
make
make test
make install
```

## Design concepts

**Data containers**

There are two main data containers: `Sequence` for 1D data, and `Image` for ND data.
Underlying storage is handled by Kokkos by default, and adapts to the target infrastructure.
There is generally no memory ordering or contiguity guarantee for `Image` objects.
In return, execution is automatically parallelized by Kokkos, including on GPU.

For interfacing with libraries which require contiguity,
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
* by associating a `Region` to a data class with `where()`, which results in an object of type `Patch`.

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

## Alternatives

The following libraries offer features similar to Linx.
Linx aims at being simpler, less verbose, more extensible, and natively GPU-compatible
although with a more limited feature set.

* Armadillo
* Blitz++
* Boost.MultiArray
* CImg
* Eigen
* ITK, SimpleITK
* ndarray
* OpenCV
* STL's valarray
* XTensor

Here is a quick comparison of ITK, CImg, Linx and NumPy/SciKit for the following use case:
read an image, dilate it with an L2-ball structuring element, and write the output.

**ITK**

```cpp
using T = unsigned char;
static constexpr unsigned int N = 2;
using Image = itk::Image<T, N>;

auto raw = itk::ReadImage<ImageType>(input);

using StructuringElement = itk::FlatStructuringElement<N>;
StructuringElement::RadiusType strelRadius;
strelRadius.Fill(radius);
StructuringElementType ball = StructuringElement::Ball(strelRadius);
using GrayscaleDilateImageFilter = itk::GrayscaleDilateImageFilter<Image, Image, StructuringElement>;
GrayscaleDilateImageFilter::Pointer dilateFilter = GrayscaleDilateImageFilter::New();
dilateFilter->SetInput(input);
dilateFilter->SetKernel(ball);

itk::WriteImage(dilateFilter->GetOutput(), output);
```

**CImg** (limited to N <= 3)

```cpp
using T = unsigned char;

auto raw = cimg::CImg<T>().load(input);

cimg::CImg<bool> ball(2 * radius + 1, 2 * radius + 1, 2 * radius + 1, 1, false);
bool color[1] = {true};
ball.draw_circle(radius, radius, radius, color);
auto dilated = raw.get_dilate(ball, 0, true);

dilated.write(output);
```

**Linx**

```cpp
using T = unsigned char;
static constexpr Linx::Index n = 2;

auto raw = Linx::read<T, n>(input);

auto ball = Linx::Mask<n>::ball<2>(radius);
auto dilated = Linx::Dilation(ball) * raw;

Linx::write(dilated, output);
```

**NumPy/SciKit**

```python
raw = np.load(input)

ball = skimage.morphology.disk(radius)
dilated = skimage.morphology.dilation(raw, ball)

np.save(output, dilated)
```
