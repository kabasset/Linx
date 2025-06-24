# Project overview

<br/>![Linx logo](doc/diagrams/logo_square.svg)

## Introduction

Linx is a package for parallel multidimensional signal and image processing.
It focuses on performance portability, i.e. the ability to write a single code able to perform efficiently on a variety of targets, including many-core processors and GPUs.
Built on top of Kokkos, Linx provides a higher-level interface for writing parallel processing workflows,
using arithmetic operations, linear and non-linear filtering, random number generation, and more.

## License

Linx is licensed under [Apache-2.0](LICENSE.txt).

## Build

Linx relies on C++20 features.
It depends on Kokkos and CFITSIO.
While CFITSIO binaries are available, Kokkos has to be built from sources for performance.
Kokkos' `Serial` execution space is mandatory, while `OpenMP` and `Cuda` backends are optional.
Other backends have not been tested.

The following lines will clone and install Kokkos and Linx in given directories.

```sh
export CLONE_DIR=<clone_dir>
export KOKKOS_INSTALL_DIR=<kokkos_install_dir>
export LINX_INSTALL_DIR=<linx_install_dir>

cd $CLONE_DIR
git clone https://github.com/kokkos/kokkos.git
cd kokkos
export KOKKOS_SOURCE_DIR=$PWD

mkdir build
cd build
cmake $KOKKOS_SOURCE_DIR -DCMAKE_CXX_STANDARD=20 \
  -DCMAKE_INSTALL_PREFIX=$KOKKOS_INSTALL_DIR \
  -DKokkos_ENABLE_SERIAL=ON \
  [-DKokkos_ENABLE_OPENMP=ON] \
  [-DKokkos_ENABLE_CUDA=ON -DKokkos_ENABLE_CUDA_CONSTEXPR=ON -DCMAKE_CXX_COMPILER=$KOKKOS_SOURCE_DIR/bin/nvcc_wrapper]
make install

cd $CLONE_DIR
git clone https://github.com/kabasset/Linx.git
cd Linx
export LINX_SOURCE_DIR=$PWD

mkdir build
cd build
cmake .. -DKokkos_ROOT=$KOKKOS_INSTALL_DIR -DCMAKE_INSTALL_PREFIX=$LINX_INSTALL_DIR
make
make test
make install
```

## API overview

**Data containers**

There are two main data containers: `Sequence` for 1D arrays, and `Image` for ND arrays.
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

Arbitrarily complex functions can also be applied pointwise with `transform()` or `generate()`:

```cpp
auto a = Linx::Image(...);
auto b = Linx::Image(...);
a.generate(
    "random noise",
    Linx::GaussianRng());
a.transform(
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
    KOKKOS_LAMBDA(auto b_i, auto c_i) { return Kokkos::sqrt(b_i * c_i); },
    b, c);
```

**Global transforms**

Global transforms such as Fourier transforms and convolutions are also supported.

```cpp
auto a = Linx::Image(...);
auto k = Linx::Image(...);
auto b = Linx::Correlation(k)(a); // Creates a new instance
auto c = Linx::Image(...);
Linx::Correlation(k).transform(a, c); // Fills c
```

**Regional transforms**

There are two ways to work on subsets of elements:
* by slicing some data classes with operator `[]`, which returns a view of type `Sequence` or `Image` depending on the input type;
* by associating a region to a data class as an object of type `Patch`.

Slices are created from regions of type either `Slice` or `Box`.

Patches accept any type of region, are extremely lightweight and can be moved around when the region is a window, i.e. has shifting capabilities.
Typical windows are `Box`, `Mask` or `Path` and can be used to perform operations locally.
As opposed to slicing, patching results in an object of type `Patch` instead of simply `Sequence` or `Image`.
Nevertheless, patches are themselves data containers and can be transformed pointwise:

```cpp
auto image = Linx::Image(...):
auto region = Linx::Box(...);
auto slice = image[region];
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
using Image = itk::Image<int, 2>;

auto raw = itk::ReadImage<Image>(filename);

using StructuringElement = itk::FlatStructuringElement<2>;
using GrayscaleDilateImageFilter = itk::GrayscaleDilateImageFilter<Image, Image, StructuringElement>;

StructuringElement::RadiusType strelRadius;
strelRadius.Fill(radius);
StructuringElementType ball = StructuringElement::Ball(strelRadius);
GrayscaleDilateImageFilter::Pointer dilateFilter = GrayscaleDilateImageFilter::New();
dilateFilter->SetInput(raw);
dilateFilter->SetKernel(ball);

itk::WriteImage(dilateFilter->GetOutput(), output);
```

**CImg** (limited to N <= 3)

```cpp
auto raw = cimg::CImg<int>().load(filename);

cimg::CImg<bool> ball(2 * radius + 1, 2 * radius + 1, 2 * radius + 1, 1, false);
bool color[1] = {true};
ball.draw_circle(radius, radius, radius, color);
auto dilated = raw.get_dilate(ball, 0, true);

dilated.write(output);
```

**Linx**

```cpp
auto raw = Linx::read<int, 2>(filename);

auto ball = Linx::Mask<2>::ball(radius);
auto dilated = Linx::Dilation(ball).pad(0)(raw);

Linx::write(dilated, output);
```

**NumPy/SciKit**

```python
raw = np.load(filename)

ball = skimage.morphology.disk(radius)
dilated = skimage.morphology.dilation(raw, ball)

np.save(output, dilated)
```
